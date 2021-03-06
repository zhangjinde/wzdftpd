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

#ifndef WIN32
#include <unistd.h>
#endif

#include <libwzd-core/wzd_structs.h>
#include <libwzd-core/wzd_libmain.h>
#include <libwzd-core/wzd_log.h>

#include <libwzd-core/wzd_mod.h>

#include "debug_backends.h"
#include "debug_commands.h"
#include "debug_context.h"
#include "debug_crontab.h"
#include "debug_modules.h"

typedef struct {
  const char * name;
  wzd_function_command_t fct;
} dbg_command_name_t;

int add_debug_commands(void)
{
  dbg_command_name_t commands[] = {
    { "site_cronjob", do_site_cronjob },
    { "site_listbackends", do_site_listbackends },
    { "site_listcontexts", do_site_listcontexts },
    { "site_listcrontab", do_site_listcrontab },
    { "site_listmodules", do_site_listmodules },
    { NULL, NULL }
  };
  int i;

  for (i=0; commands[i].name != NULL; i++) {
    if (commands_add(getlib_mainConfig()->commands_list,commands[i].name,commands[i].fct,NULL,TOK_CUSTOM)) {
      out_log(LEVEL_HIGH,"ERROR while adding custom command: %s\n",commands[i].name)
        ;
      return -1;
    }

    /* default permission XXX hardcoded */
    if (commands_set_permission(getlib_mainConfig()->commands_list,commands[i].name, "+O")) {
      out_log(LEVEL_HIGH,"ERROR setting default permission to custom command %s\n",commands[i].name);
      /** \bug XXX remove command from   config->commands_list */
      return -1;
    }
  }

  return 0;
}

