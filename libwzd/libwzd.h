/* vi:ai:et:ts=8 sw=2
 */
/*
 * wzdftpd - a modular and cool ftp server
 * Copyright (C) 2002-2004  Pierre Chifflier
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

/** \file libwzd.h
 *  \brief Routines to access wzdftpd from applications
 */

#ifndef __LIBWZD__
#define __LIBWZD__

/** parameters are still being defined
 *
 * wzd_init: connect to server
 * 
 */
int wzd_init(const char *host, int port, const char *user, const char *pass);

int wzd_fini(void);

int wzd_send_message(const char *buffer, int length, char * reply, int reply_length);

/* TODO missing functions:
 *
 * - disconnect
 * - send_command(const char *)
 *     |-> send_command should check connection status and re-connect if needed
 *
 * shortcuts to send_command: site_who, kick, kill, stop_server, etc.
 */

#endif /* __LIBWZD__ */
