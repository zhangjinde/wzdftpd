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

/** \file hash.c
  * \brief Hash tables implementation
  */

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hash.h"

typedef struct {
  void *key;
  void *data;
  htrigger_function trigger; /* trigger function to be called on modification */
  hfree free_key;
  hfree free_element;
} CHTBL_Elmnt;


unsigned int hash_str(const char *key)
{
  unsigned hashval;

  for (hashval = 0; *key != '\0'; key++)
    hashval = (*key * 7 + hashval) % 781;
/*    hashval = *key + 31 * hashval;*/
  return hashval;
}


int chtbl_init(CHTBL *htab, int containers, int (*h)(const void*),
    int (*match)(const void*, const void*),
    void (*ffree)(void*))
{
  int i;

  if ((htab->table = (List*)malloc(containers * sizeof(List))) == NULL)
    return -1;

  htab->containers = containers;

  for (i=0; i<containers; i++)
    list_init(&htab->table[i], free);

  htab->h = h;
  htab->match = match;
  htab->free = ffree;

  htab->size = 0;

  return 0;
}

void chtbl_destroy(CHTBL *htab)
{
  int i;

  for (i=0; i<htab->containers; i++)
  {
    CHTBL_Elmnt *data;
    List * list;

    list = &htab->table[i];

/*    list_destroy(&htab->table[i]);*/

    while (list_size(list) > 0) {
      if (list_rem_next(list,0,(void**)&data)==0 && list->destroy != NULL) {
        if (data->free_key) data->free_key(data->data);
        if (data->free_element) data->free_element(data->data);
        list->destroy(data);
      }
    }
  }

  free(htab->table);

  memset(htab, 0, sizeof(CHTBL));
}

int chtbl_insert(CHTBL *htab, const void *key, void *data, htrigger_function fcn, hfree free_key, hfree free_element)
{
  CHTBL_Elmnt *entry;
  int index;
  int ret;

  /* already present ? */
  if (chtbl_lookup(htab, key, NULL) == 0)
    return 1;

  index = htab->h(key) % htab->containers;

  /* insertion */
  entry = malloc(sizeof(CHTBL_Elmnt));
  entry->key = (void*)key;
  entry->data = data;
  entry->trigger = fcn;
  entry->free_key = free_key;
  entry->free_element = free_element;

  if ((ret = list_ins_next(&htab->table[index], NULL, entry)) == 0)
    htab->size++;
  else
    free(entry);

  return ret;
}

int chtbl_remove(CHTBL *htab, const void *key)
{
  ListElmt *prec, *element;
  CHTBL_Elmnt *entry;
  int index;

  index = htab->h(key) % htab->containers;

  prec = NULL;
  for (element = list_head(&htab->table[index]); element != NULL; element = list_next(element))
  {
    entry = list_data(element);
    if (!entry) return -1;
    if (htab->match(key, entry->key)==0) {
      if (list_rem_next(&htab->table[index], prec, (void*)entry) == 0) {
        htab->size--;
        free(entry);
        return 0;
      }
      return -1;
    }
    prec = element;
  }

  return -1;
}

int chtbl_lookup(const CHTBL *htab, const void *key, void **data)
{
  ListElmt *element;
  CHTBL_Elmnt *entry;
  int index;

  index = htab->h(key) % htab->containers;

  for (element=list_head(&htab->table[index]); element != NULL; element = list_next(element))
  {
    entry = list_data(element);
    if (!entry) return -1;
    if (htab->match(key, entry->key)==0) {
      if (data) *data = entry->data;
      return 0;
    }
  }

  return 1;
}

int chtbl_change(const CHTBL *htab, const void *key, void *data)
{
  ListElmt *element;
  CHTBL_Elmnt *entry;
  int index;
  int ret;

  index = htab->h(key) % htab->containers;

  for (element=list_head(&htab->table[index]); element != NULL; element = list_next(element))
  {
    entry = list_data(element);
    if (!entry) return -1;
    if (htab->match(key, entry->key)==0) {
      if (data) entry->data = data;
      if (entry->trigger) {
        ret = (entry->trigger)(entry->key,entry->data);
      }
      return 0;
    }
  }

  return 1;
}

int chtbl_insert_or_change(CHTBL *htab, const void *key, void *data, htrigger_function fcn, hfree free_key, hfree free_element)
{
  if(chtbl_insert(htab, key, data, fcn, free_key, free_element)) {
    return chtbl_change(htab, key, data);
  }
  return 0;
}