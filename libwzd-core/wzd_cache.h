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

#ifndef __WZD_CACHE__
#define __WZD_CACHE__

#include "wzd_structs.h"

struct wzd_cache_t;
typedef struct wzd_cache_t wzd_cache_t;

/** \brief Open file and put it in cache.
 *
 * \note Initial refcount is 2, so file will not be freed on the first closing.
 */
wzd_cache_t* wzd_cache_open(const char *file, int flags, unsigned int mode);

/** force update of specific file, only if present in cache */
void wzd_cache_update(const char *file);

/** \brief Get size of file */
off_t wzd_cache_getsize(wzd_cache_t *c);

/** \brief Read data from file */
ssize_t wzd_cache_read(wzd_cache_t * c, void *buf, size_t count);

/** \brief Write data to file */
ssize_t wzd_cache_write(wzd_cache_t * c, void *buf, size_t count);

/** \brief Attempt to read a line from file, reading at most \a size characters */
char * wzd_cache_gets(wzd_cache_t * c, char *buf, unsigned int size);

/** \brief Decrement reference count on file. If 0, file is closed */
void wzd_cache_close(wzd_cache_t * c);

/** \brief Purge all files in cache */
void wzd_cache_purge(void);

/** \brief Open file in cache, read it and return contents */
int wzd_cache_read_file_fast(const char * filename, char ** buffer, size_t * size);


#ifndef WZD_NO_USER_CACHE

typedef int (*predicate_user_t)(wzd_user_t *, void * arg);
typedef int (*predicate_group_t)(wzd_group_t *, void * arg);

int predicate_name(wzd_user_t * user, void * arg);
int predicate_groupname(wzd_group_t * group, void * arg);

void usercache_init(void);
void usercache_fini(void);

wzd_user_t * usercache_add(wzd_user_t * user);
wzd_user_t * usercache_getbyname( const char * name );
wzd_user_t * usercache_getbyuid( unsigned int uid );
wzd_user_t * usercache_search( predicate_user_t p, void * arg );
void usercache_invalidate( predicate_user_t p, void * arg );

wzd_group_t * groupcache_add(wzd_group_t * group);
wzd_group_t * groupcache_getbyname( const char * name );
wzd_group_t * groupcache_getbygid( unsigned int gid );
wzd_group_t * groupcache_search( predicate_group_t p, void * arg );
void groupcache_invalidate( predicate_group_t p, void * arg );

#endif /* WZD_NO_USER_CACHE */

#endif /* __WZD_CACHE__ */

