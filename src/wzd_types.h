/*
 * wzdftpd - a modular and cool ftp server
 * Copyright (C) 2002-2003  Pierre Chifflier
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

/* wzd_types.h : system types */
#ifndef __WZD_TYPES__
#define __WZD_TYPES__

#include <sys/types.h>


#if defined(_MSC_VER)

#define HARD_USERLIMIT	128

/* unsigned int, 64 bits: u64_t */
#define u64_t unsigned __int64
#include <sys/timeb.h>


#define F_RDLCK 0 /* Read lock. */
#define F_WRLCK 1 /* Write lock. */
#define F_UNLCK 2 /* Remove lock. */

#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define	LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7

#define __S_ISTYPE(mode,mask) (((mode) & _S_IFMT) == (mask))

#define S_ISDIR(mode) __S_ISTYPE((mode), _S_IFDIR)
#define S_ISLNK(mode) 0
#define S_ISREG(mode) __S_ISTYPE((mode), _S_IFREG)

#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC
#define S_IRGRP S_IREAD
#define S_IWGRP S_IWRITE
#define S_IXGRP S_IEXEC
#define S_IROTH S_IREAD
#define S_IWOTH S_IWRITE
#define S_IXOTH S_IEXEC

#define chmod	_chmod

#define dlopen(filename,dummy)	LoadLibrary(filename)
#define dlclose(handle)			FreeLibrary(handle)
#define dlsym(handle,symbol)	GetProcAddress(handle,symbol)
#define dlerror()				"Not supported on win32"

#define getcwd	_getcwd

/* FIXME this will surely have some effects ... */
#define lstat	stat

#define mkdir(filename,mode)	_mkdir(filename)
#define closedir	FindClose

#define open	_open

#define popen	_popen
#define pclose	_pclose

#define readlink(path,buf,bufsiz)	(-1)
#define symlink(oldpath,newpath)	(-1)

#define strcasecmp	stricmp
#define strncasecmp	strnicmp

#define snprintf	_snprintf
#define vsnprintf	_vsnprintf

#define pid_t		unsigned int
#define socklen_t	unsigned int
#define uid_t		unsigned int


/*********************** VERSION **************************/

/* Version */
#define       WZD_VERSION_NUM "0.1 visual"
#define  WZD_BUILD_NUM 20031025
#define  WZD_BUILD_OPTS  "visual"

#ifdef WZD_MULTIPROCESS
#define WZD_MP  " mp "
#else /* WZD_MULTIPROCESS */
#ifdef WZD_MULTITHREAD
#define WZD_MP  " mt "
#else
#define WZD_MP  " up "
#endif /* WZD_MULTITHREAD */
#endif /* WZD_MULTIPROCESS */

#ifdef __CYGWIN__
#define WZD_VERSION_STR "wzdftpd cygwin" WZD_MP WZD_VERSION_NUM
#else /* __CYGWIN__ */
#define WZD_VERSION_STR "wzdftpd linux" WZD_MP WZD_VERSION_NUM
#endif /* __CYGWIN__ */

#define WZD_DEFAULT_CONF "wzd-win32.cfg"

#include "wzd_crypt.h"
#include "wzd_md5crypt.h"
#include "wzd_strptime.h"
#include "wzd_strtok_r.h"
#include "wzd_strtoull.h"

#else /* _MSC_VER */

/* unsigned int, 64 bits: u64_t */
#define u64_t u_int64_t
#include <sys/time.h> /* struct timeval */

#endif /* _MSC_VER */

#endif /* __WZD_TYPES__ */