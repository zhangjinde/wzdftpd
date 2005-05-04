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

#ifndef __LIBPGSQL__
#define __LIBPGSQL__

enum {
  UCOL_REF=0,
  UCOL_USERNAME,
  UCOL_USERPASS,
  UCOL_ROOTPATH,
  UCOL_TAGLINE,
  UCOL_UID,
  UCOL_FLAGS,
  UCOL_MAX_IDLE_TIME,
  UCOL_MAX_UL_SPEED,
  UCOL_MAX_DL_SPEED,
  UCOL_NUM_LOGINS,
  UCOL_RATIO,
  UCOL_USER_SLOTS,
  UCOL_LEECH_SLOTS,
  UCOL_PERMS,
  UCOL_CREDITS,
  UCOL_LAST_LOGIN,
};

enum {
  GCOL_REF=0,
  GCOL_GROUPNAME,
  GCOL_GID,
  GCOL_DEFAULTPATH,
  GCOL_TAGLINE,
  GCOL_GROUPPERMS,
  GCOL_MAX_IDLE_TIME,
  GCOL_NUM_LOGINS,
  GCOL_MAX_UL_SPEED,
  GCOL_MAX_DL_SPEED,
  GCOL_RATIO,
};

enum {
  UIPCOL_REF=0,
  UIPCOL_IP,
};

extern PGconn * pgconn;

void _wzd_pgsql_error(const char *filename, const char  *func_name, int line); /*, const char *error); */

int _wzd_run_delete_query(char * query, size_t length, const char * query_format, ...);
int _wzd_run_insert_query(char * query, size_t length, const char * query_format, ...);
int _wzd_run_update_query(char * query, size_t length, const char * query_format, ...);

/* basic syntax checking to avoid injections */
int wzd_pgsql_check_name(const char *name);

char * _append_safely_mod(char *query, unsigned int *query_length, char *mod, unsigned int modified);

gid_t FCN_FIND_GROUP(const char *name, wzd_group_t * group);
int wpgsql_mod_group(const char *name, wzd_group_t * group, unsigned long mod_type);
int wpgsql_mod_user(const char *name, wzd_user_t * user, unsigned long mod_type);

#endif /* __LIBPGSQL__ */
