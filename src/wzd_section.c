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

#include <sys/time.h>	/* time_t (wzd_structs.h) */
#include <arpa/inet.h>	/* struct in_addr (wzd_misc.h) */

#include <sys/stat.h>
#include <malloc.h>	/* malloc/free */
#include <string.h>	/* strdup */

/* speed up compilation */
#define SSL	void
#define SSL_CTX	void
#define FILE	void

#include "wzd_structs.h"

#include "wzd_section.h"
#include "wzd_misc.h"


struct wzd_section_t {
  char *        sectionname;
  char *        sectionmask;

  struct wzd_section_t * next_section;
};

int section_add(wzd_section_t **section_list, unsigned char *name, unsigned char *mask)
{
  wzd_section_t * section_new, * section;

  if (!section_list || !name || !mask) return -1;

  section_new = malloc(sizeof(wzd_section_t));
  section_new->sectionname = strdup(name);
  section_new->sectionmask = strdup(mask);
  section_new = NULL;

  section = *section_list;

  /* head insertion ? */
  if (!section) {
    *section_list = section_new;
    return 0;
  }

  do {
    /* do not insert if a section with same name exists */
    if (strcmp(name,section->sectionname)==0) return 1;
    /* FIXME if a section with same or bigger mask exist, warn user ? */
    section = section->next_section;
  }
  while ( section->next_section );

  section->next_section = section_new;

  return 0;
}

int section_free(wzd_section_t **section_list)
{
  wzd_section_t * section, * section_next;

  if (!section_list) return 0;
  section = *section_list;

  while (section)
  {
    section_next = section->next_section;
    free(section->sectionname);
    free(section->sectionmask);
    free(section);
    section = section_next;
  }
  *section_list = NULL;

  return 0;
}

/** \return 1 if in section, else 0 */
int section_check(wzd_section_t * section, const char *path)
{
  /* TODO we can restrict sections to users/groups, etc */
  if (my_str_compare(path, section->sectionmask)) return 1;
  return 0;
}