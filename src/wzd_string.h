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

#ifndef __WZD_STRING__
#define __WZD_STRING__

typedef struct wzd_string_t wzd_string_t;

wzd_string_t * str_allocate(void);
void str_deallocate(wzd_string_t *st);


/** returns a pointer to a new string which is a duplicate of the string str.
 */
wzd_string_t * str_fromchar(const char *str);

#define STR(x) str_fromchar((x))

/** returns a pointer to the data contained in the string str.
 * These data must NOT be modified !
 */
const char * str_tochar(const wzd_string_t *str);



/** \brief returns a pointer to a new string which is a duplicate of the string src.
 */
wzd_string_t * str_dup(const wzd_string_t *src);

/** \brief copies the string pointed to by src (including the terminating `\\0'
 * character) to the array pointed to by  dest.
 */
wzd_string_t * str_copy(wzd_string_t *dst, const wzd_string_t *src);

/** \brief append 'tail' to string pointed to by str
 */
wzd_string_t * str_append(wzd_string_t * str, const char *tail);

/** \brief prepend 'head' to string pointed to by str
 */
wzd_string_t * str_prepend(wzd_string_t * str, const char *head);

/** \brief remove all leading and trailing spaces from input string
 */
wzd_string_t * str_trim(wzd_string_t * str);
wzd_string_t * str_trim_left(wzd_string_t *str);
wzd_string_t * str_trim_right(wzd_string_t *str);

/** \brief Convert string to lower case
 * \note
 * This function modifies its input string
 */
wzd_string_t * str_tolower(wzd_string_t *str);

/** \brief Extract token from string str
 * \note
 * This function modifies its input string
 */
wzd_string_t * str_tok(wzd_string_t *str, const char *delim);

/** \brief Produce output according to format and variable number of arguments,
 * and write output to str.
 */
int str_sprintf(wzd_string_t *str, const char *format, ...);

/** \brief Convert utf8 string to other charset
 * \note
 * Require unicode support
 */
int str_utf8_to_local(wzd_string_t *str, const char * charset);


/******* XXX to be implemented XXX **********/

#endif /* __WZD_STRING__ */

