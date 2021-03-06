/* vi:ai:et:ts=8 sw=2
 */
/*
 * wzdftpd - a modular and cool ftp server
 * Copyright (C) 2002-2008  Pierre Chifflier
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Pierre Chifflier
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/** \file libwzd_socket.c
 *  \brief network sockets help functions
 *
 *  Use sockets to connect to server, using standard FTP protocol.
 *  + does not require any special configuration on server
 *  + work everywhere
 *  - client showed in SITE WHO
 *  - clear connection, unless using SSL/TLS
 *  - risks of deconnection
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "libwzd.h"
#include "libwzd_err.h"
#include "libwzd_pv.h"

#include "libwzd_socket.h"
#include "libwzd_tls.h"

#ifndef WIN32
# include <unistd.h>
# include <netinet/in.h> /* struct sockaddr_in */
# include <netdb.h> /* gethostbyname */
# include <sys/socket.h>
#else
# include <winsock2.h>
# include <io.h> /* _close */
#endif /* WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static int socket_connect4(const char *host, int port, const char *user, const char *pass);
static int socket_disconnect(void);
static int socket_read(char *buffer, int length);
static int socket_write(const char *buffer, int length);
static int socket_is_secure(void);

#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
static int socket_tls_switch(void);
#endif


int server_try_socket(void)
{
  char * buffer=NULL;
  int ret;

  if (!_config) return -1;

  if (!tls_init()) _config->options &= ~OPTION_TLS;

  _config->connector.mode = CNT_SOCKET;
  /** \todo create a wrapper function to detect if _config->host is IPv6, if it has
   * multiples addresses etc. and try to connect in order.
   */
  _config->connector.connect = &socket_connect4;
  _config->connector.disconnect = &socket_disconnect;
  _config->connector.read = &socket_read;
  _config->connector.write = &socket_write;
  _config->connector.is_secure = &socket_is_secure;

  _config->state = STATE_CONNECTING;

  /* connected */
  _config->sock = _config->connector.connect(_config->host,_config->port,_config->user,_config->pass);
  if (_config->sock < 0) goto server_try_socket_abort;

  buffer = malloc(1024);

  /* read welcome message (220) */
  ret = _config->connector.read(buffer,1024);
  if (ret <= 0) goto server_try_socket_abort;
  if (ret > 0) {
    buffer[ret] = '\0';
  }
  if (buffer[0] != '2' || buffer[1] != '2')
    goto server_try_socket_abort;

  /* TLS mode ? */
#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
  if ( !(_config->options & OPTION_NOTLS) ) {
    ret = socket_tls_switch();
    if (ret < 0 && OPTION_TLS) {
      err_store("Could not switch to TLS");
      goto server_try_socket_abort; /* abort only if TLS was forced */
    }
  }
#endif /* HAVE_GNUTLS */

  /* USER name */
  snprintf(buffer,1024,"USER %s\r\n",_config->user);
  ret = _config->connector.write(buffer,strlen(buffer));
  if (ret < 0 || ret != (int)strlen(buffer))
    goto server_try_socket_abort;

  /* 331 User name okay, need password. */
  ret = _config->connector.read(buffer,1024);
  if (ret <= 0) goto server_try_socket_abort;
  if (ret > 0) {
    buffer[ret] = '\0';
  }
  if (buffer[0] != '3' || buffer[1] != '3' || buffer[2] != '1')
    goto server_try_socket_abort;

  /* PASS xxx */
  snprintf(buffer,1024,"PASS %s\r\n",_config->pass);
  ret = _config->connector.write(buffer,strlen(buffer));
  if (ret < 0 || ret != (int)strlen(buffer))
    goto server_try_socket_abort;

  /* 230 User logged in, proceed. */
  ret = _config->connector.read(buffer,1024);
  if (ret <= 0) goto server_try_socket_abort;
  if (ret > 0) {
    buffer[ret] = '\0';
  }
  if (buffer[0] != '2' || buffer[1] != '3' || buffer[2] != '0')
    goto server_try_socket_abort;

  /* go into ghost mode ? */

  _config->state = STATE_OK;

  return _config->sock;

server_try_socket_abort:
#ifdef DEBUG
  if (buffer != NULL)
    fprintf(stderr,"last message was: [%s]\n",buffer);
#endif
  free(buffer);
  _config->connector.disconnect();
  _config->connector.disconnect = NULL;
  _config->connector.read = NULL;
  _config->connector.write = NULL;
  _config->state = STATE_NONE;
  return -1;
}



static int socket_connect4(const char *host, int port, const char *user, const char *pass)
{
  struct sockaddr_in sai;
  struct hostent* host_info;
  int sock;
#ifndef WIN32
  int i;
#endif
  int ret;

  if (!_config) return -1;

  if( (host_info = gethostbyname(host)) == NULL)
  {
    return -1;
  }
  memcpy(&sai.sin_addr, host_info->h_addr, host_info->h_length);

  sock = socket(PF_INET,SOCK_STREAM,0);
  if (sock < 0) return -1;

#ifndef WIN32
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&i,sizeof(i));
#endif

  /* make connection */
  sai.sin_port = htons((unsigned short)port);
  sai.sin_family = AF_INET;

  /* set non-blocking mode ? */
  ret = connect(sock,(struct sockaddr*)&sai,sizeof(sai));
  if (ret < 0) {
    close(sock);
    return -1;
  }

  /* try to switch to tls/ssl ?
   * not now, (explicit)
   */

  return sock;
}

static int socket_disconnect(void)
{
  if (!_config) return -1;

  if (_config->sock < 0) return -1;
#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
  if (_config->options & OPTION_TLS) {
    tls_deinit();
  }
#endif
  close(_config->sock);
  _config->sock = -1;

  memset( &(_config->connector), 0, sizeof(_config->connector) );

  return 0;
}

/* XXX FIXME
 * this function should decode the FTP protocol, that means:
 * read the 3 first bytes, find reply code,
 * find if multi-line reply (4th byte is '-', if yes find sequence
 * code[space] at beginning of line OR inside reply (\r\ncode[space])
 *
 * the end_seq should be constructed only at first loop (we should separate
 * the first loop from multi-line case ?)
 */
static int socket_read(char *buffer, int length)
{
  unsigned int offset=0;
  int ret=0;
  char end_seq[5];

  if (!_config) return -1;
  if (_config->sock < 0) return -1;

  do {
    offset += ret;

#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
    if (_config->options & OPTION_TLS) {
      ret = tls_read( buffer+offset, length-offset );
    } else
#endif
    ret = read( _config->sock, buffer+offset, length-offset );

    if (ret < 0) {
      /** \todo detect if connection was close, and either
       * 1/ try to reopen it
       * 2/ set _config->state to STATE_ERROR
       */
      return -1;
    }

    /* end ? */
    if (ret < 4) continue;

    /* check validity ? */
#if 0
    if ( buffer[offset] < '0' || buffer[offset] > '9'
        || buffer[offset+1] < '0' || buffer[offset+1] > '9'
        || buffer[offset+2] < '0' || buffer[offset+2] > '9')
      continue;
#endif

    if (buffer[offset+3] == ' ') break;

    memcpy(end_seq,buffer,3);
    end_seq[3] = ' ';
    end_seq[4] = '\0';
    if (strstr(buffer,end_seq)) break;

/*    break;*/

  } while (1);

  offset += ret;

  return offset;
}

/* XXX FIXME
 * This function should ensure command is really RFC-compliant
 * (one ligne only, ended with \r\n)
 */
static int socket_write(const char *buffer, int length)
{
  char * send_buffer;
  int ret;

  if (!_config) return -1;
  if (_config->sock < 0) return -1;
  if (length < 0) return -1;

  send_buffer = malloc(length+3);
  snprintf(send_buffer,length+3,"%s\r\n",buffer);

#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
  if (_config->options & OPTION_TLS) {
    ret = tls_write(send_buffer, length);
  } else
#endif
  ret = write(_config->sock, send_buffer, length);
  /** \todo detect if connection was close, and either
   * 1/ try to reopen it
   * 2/ set _config->state to STATE_ERROR
   */
  free(send_buffer);
  return ret;
}

static int socket_is_secure(void)
{
  if (!_config) return 0; /* NOT secure */
  return ( (_config->options & OPTION_TLS) ? 1 : 0 );
}

#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
static int socket_tls_switch(void)
{
  char * buffer;
  int ret;

  if (!_config) return -1;
  if ( (_config->options & OPTION_NOTLS) ) return -1; /* we don't want TLS */
  if (_config->sock < 0) return -1;

  buffer = malloc(1024);

  /* AUTH TLS */
  snprintf(buffer,1024,"AUTH TLS\r\n");
  ret = _config->connector.write(buffer,strlen(buffer));
  if (ret < 0 || ret != (int)strlen(buffer))
    goto socket_tls_switch_abort;

  /* 234 234 AUTH command OK. Initializing TLS mode */
  ret = _config->connector.read(buffer,1024);
  if (ret <= 0) goto socket_tls_switch_abort;
  if (ret > 0) {
    buffer[ret] = '\0';
  }
  if (buffer[0] != '2' || buffer[1] != '3' || buffer[2] != '4')
    goto socket_tls_switch_abort;

  tls_handshake(_config->sock);
  if (ret < 0) goto socket_tls_switch_abort; /* ... */

  _config->options |= OPTION_TLS;

  return 0;

socket_tls_switch_abort:
  free(buffer);
  _config->options &= ~OPTION_TLS;
  return -1;
}
#endif

