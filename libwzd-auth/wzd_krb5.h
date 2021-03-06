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

#ifndef __WZD_KRB5__
#define __WZD_KRB5__

/*! \file wzd_krb5.h
 * \brief Implementation of the GSSAPI support (as described in RFC 2228)
 *
 * \addtogroup libwzd_auth
 *  @{
 */

typedef struct _auth_gssapi_data_t * auth_gssapi_data_t;

/** \brief Initialize GSSAPI code
 *
 * \note This function should be called only once
 */
int auth_gssapi_init(auth_gssapi_data_t * data);

/** \brief Accept security context
 */
int auth_gssapi_accept_sec_context(auth_gssapi_data_t data, char * ptr_in,size_t length_in, char ** ptr_out, size_t * length_out);

/** \brief Decode MIC (or ENC) command
 *
 * Command is decoded from base64, then passed to gss_unwrap
 */
int auth_gssapi_decode_mic(auth_gssapi_data_t data, char * ptr_in,size_t length_in, char ** ptr_out, size_t * length_out);

/** \brief Encode GSSAPI reply
 *
 * Reply must be stripped from the last CRLF, then passed to gss_wrap and encoded in base 64. A reply code is prepended:
 *  - 631 for a reply ensuring integrity only
 *  - 632 for a reply ensuring integrity and confidentiality
 *  - 633 for a reply ensuring confidentiality only
 */
int auth_gssapi_encode(auth_gssapi_data_t data, char * ptr_in,size_t length_in, char ** ptr_out, size_t * length_out);

/** \brief Read function for a GSSAPI-compliant client
 *
 * Check if command is protected by MIC or ENC
 */
int auth_gssapi_read(int sock, char *msg, size_t length, int flags, unsigned int timeout, void * vcontext);

/** \brief Write function for a GSSAPI-compliant client
 *
 * Encode reply before sending
 */
int auth_gssapi_write(int sock, const char *msg, size_t length, int flags, unsigned int timeout, void * vcontext);

/* return 1 if user is validated */
int check_krb5(const char *user, const char *data);

int changepass_krb5(const char *pass, char *buffer, size_t len);

/*! @} */

#endif /* __WZD_KRB5__ */

