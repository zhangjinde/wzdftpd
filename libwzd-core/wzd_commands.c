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

#include "wzd_all.h"

#ifndef WZD_USE_PCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libwzd-base/hash.h>

#include "wzd_structs.h"
#include "wzd_log.h"
#include "wzd_misc.h"
#include "wzd_perm.h"
#include "wzd_site.h"
#include "wzd_site_group.h"
#include "wzd_site_user.h"
#include "wzd_ClientThread.h"

#include "wzd_debug.h"

#endif /* WZD_USE_PCH */

static void _command_free(wzd_command_t *command)
{
  if (!command) return;
  free(command->name);
  str_deallocate(command->external_command);
  perm_free_recursive(command->perms);
  free(command);
}

/** \brief Initialize storage for server commands
 *
 * \param[out] _ctable Pointer to the allocated hash table
 *
 * \return 0 if ok
 */
int commands_init(CHTBL ** _ctable)
{
  if (*_ctable) {
    commands_fini(*_ctable);
  }

  *_ctable = malloc(sizeof(CHTBL));

  if (chtbl_init(*_ctable, 128, (hash_function)hash_str, (cmp_function)strcmp, (void (*)(void*))_command_free)) {
    free(*_ctable);
    *_ctable = NULL;
    return -1;
  }

  return 0;
}

/** \brief Destroy stored commands, and free memory used for commands
 *
 * \param[in] _ctable Hash table containing commands
 */
void commands_fini(CHTBL * _ctable)
{
  if (!_ctable) return;

  chtbl_destroy(_ctable);
  free(_ctable);
  _ctable = NULL;
}

/** \brief Add a new FTP command, linked to a C function
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] name The FTP command (for ex, XCRC). For a site command, append command
 * name with a space: SITE_HELP
 * \param[in] command The function which will be executed when receiving the FTP command
 * \param[in] help A pointer to a help function (not used at the moment)
 * \param[in] id A unique identifier (32 bits unsigned integer) for the command (see \ref wzd_token_t)
 *
 * \note Command names are case insensitive, and must be valid ASCII
 */
int commands_add(CHTBL * _ctable,
    const char *name,
    wzd_function_command_t command,
    wzd_function_command_t help,
    u32_t id)
{
  wzd_command_t * com;

  if (!_ctable) return -1;
  if (!name || !command || !id) return -1;

  if (chtbl_lookup(_ctable, name, (void**)&com))
  {
    /* new entry */
    com = malloc(sizeof(wzd_command_t));
    com->name = strdup(name);
    ascii_lower(com->name,strlen(com->name));
    com->id = id;
    com->command = command;
    com->help_function = help;
    com->external_command = NULL;

    com->perms = NULL;

    if ((chtbl_insert(_ctable, com->name, com, NULL, NULL, (void(*)(void*))_command_free))==0)
    {
      return 0;
    }

    free(com->name);
    free(com);
    return -1;
  }

  return 0;
}

/** \brief Add a new FTP command, linked to an external program (for ex, a perl module)
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] name The FTP command (for ex, XCRC). For a site command, append command
 * name with a space: SITE_HELP
 * \param[in] external_command The application which will be executed when receiving the FTP command.
 * The application can use protocols (see \ref hook_add_protocol)
 *
 * \note Command names are case insensitive, and must be valid ASCII
 */
int commands_add_external(CHTBL * _ctable,
    const char *name,
    const wzd_string_t *external_command)
{
  wzd_command_t * com;

  if (!_ctable) return -1;
  if (!name || !external_command) return -1;

  /** \todo this should be done in an atomic way */
  if (chtbl_lookup(_ctable, name, (void**)&com) == 0) { /* already found, replace command */
    if ((chtbl_remove(_ctable, com->name)!=0))
    {
      out_err(LEVEL_CRITICAL,"ERROR Could not remove a previous command for %s\n",name);
      return -1;
    }
  }

  /* new entry */
  com = malloc(sizeof(wzd_command_t));
  com->name = strdup(name);
  ascii_lower(com->name,strlen(com->name));
  com->id = TOK_CUSTOM;
  com->external_command = str_dup(external_command);

  com->command = NULL;
  com->help_function = NULL;

  com->perms = NULL;

  if ((chtbl_insert(_ctable, com->name, com, NULL, NULL, (void(*)(void*))_command_free))==0)
    return 0;

  str_deallocate(com->external_command);
  free(com->name);
  free(com);
  return -1;
}

/** \brief Add default FTP commands to hash table
 *
 * \param[in] _ctable Hash table containing commands
 *
 * \return 0 if ok
 */
int commands_add_defaults(CHTBL * _ctable)
{
  if (!_ctable) return -1;

  if (commands_add(_ctable,"site",do_site,NULL,TOK_SITE)) return -1;

  if (commands_add(_ctable,"type",do_type,NULL,TOK_TYPE)) return -1;
  if (commands_add(_ctable,"port",do_port,NULL,TOK_PORT)) return -1;
  if (commands_add(_ctable,"pasv",do_pasv,NULL,TOK_PASV)) return -1;
  if (commands_add(_ctable,"eprt",do_eprt,NULL,TOK_EPRT)) return -1;
  if (commands_add(_ctable,"epsv",do_epsv,NULL,TOK_EPSV)) return -1;
  if (commands_add(_ctable,"abor",do_abor,NULL,TOK_ABOR)) return -1;
  if (commands_add(_ctable,"pwd",do_print_message,NULL,TOK_PWD)) return -1;
  if (commands_add(_ctable,"allo",do_print_message,NULL,TOK_ALLO)) return -1;
  if (commands_add(_ctable,"feat",do_print_message,NULL,TOK_FEAT)) return -1;
  if (commands_add(_ctable,"noop",do_print_message,NULL,TOK_NOOP)) return -1;
  if (commands_add(_ctable,"syst",do_print_message,NULL,TOK_SYST)) return -1;
  if (commands_add(_ctable,"rnfr",do_rnfr,NULL,TOK_RNFR)) return -1;
  if (commands_add(_ctable,"rnto",do_rnto,NULL,TOK_RNTO)) return -1;
  if (commands_add(_ctable,"cdup",do_cwd,NULL,TOK_CDUP)) return -1;
  if (commands_add(_ctable,"cwd",do_cwd,NULL,TOK_CWD)) return -1;
  if (commands_add(_ctable,"list",do_list,NULL,TOK_LIST)) return -1;
  if (commands_add(_ctable,"nlst",do_list,NULL,TOK_NLST)) return -1;
  if (commands_add(_ctable,"mlst",do_mlst,NULL,TOK_MLST)) return -1;
  if (commands_add(_ctable,"mlsd",do_mlsd,NULL,TOK_MLSD)) return -1;
  if (commands_add(_ctable,"stat",do_stat,NULL,TOK_STAT)) return -1;
  if (commands_add(_ctable,"mkd",do_mkdir,NULL,TOK_MKD)) return -1;
  if (commands_add(_ctable,"rmd",do_rmdir,NULL,TOK_RMD)) return -1;
  if (commands_add(_ctable,"retr",do_retr,NULL,TOK_RETR)) return -1;
  if (commands_add(_ctable,"stor",do_stor,NULL,TOK_STOR)) return -1;
  if (commands_add(_ctable,"appe",do_stor,NULL,TOK_APPE)) return -1;
  if (commands_add(_ctable,"rest",do_rest,NULL,TOK_REST)) return -1;
  if (commands_add(_ctable,"mdtm",do_mdtm,NULL,TOK_MDTM)) return -1;
  if (commands_add(_ctable,"size",do_size,NULL,TOK_SIZE)) return -1;
  if (commands_add(_ctable,"dele",do_dele,NULL,TOK_DELE)) return -1;
  if (commands_add(_ctable,"delete",do_dele,NULL,TOK_DELE)) return -1;
  if (commands_add(_ctable,"pret",do_pret,NULL,TOK_PRET)) return -1;
  if (commands_add(_ctable,"xcrc",do_xcrc,NULL,TOK_XCRC)) return -1;
  if (commands_add(_ctable,"xmd5",do_xmd5,NULL,TOK_XMD5)) return -1;
  if (commands_add(_ctable,"opts",do_opts,NULL,TOK_OPTS)) return -1;
  if (commands_add(_ctable,"help",do_help,NULL,TOK_HELP)) return -1;
  if (commands_add(_ctable,"quit",do_quit,NULL,TOK_QUIT)) return -1;
#if defined(HAVE_OPENSSL) || defined(HAVE_GNUTLS)
  if (commands_add(_ctable,"pbsz",do_pbsz,NULL,TOK_PBSZ)) return -1;
  if (commands_add(_ctable,"prot",do_prot,NULL,TOK_PROT)) return -1;
  if (commands_add(_ctable,"cpsv",do_pasv,NULL,TOK_CPSV)) return -1;
  if (commands_add(_ctable,"sscn",do_sscn,NULL,TOK_SSCN)) return -1;
#endif
  if (commands_add(_ctable,"moda",do_moda,NULL,TOK_MODA)) return -1;

#if defined(HAVE_KRB5)
  if (commands_add(_ctable,"mic",do_mic,NULL,TOK_MIC)) return -1;
#endif

  if (commands_add(_ctable,"site_addip",do_site_addip,do_site_help_addip,TOK_SITE_ADDIP)) return -1;
  if (commands_add(_ctable,"site_adduser",do_site_adduser,do_site_help_adduser,TOK_SITE_ADDUSER)) return -1;
  if (commands_add(_ctable,"site_backend",do_site_backend,NULL,TOK_SITE_BACKEND)) return -1;
  if (commands_add(_ctable,"site_chacl",do_site_chacl,NULL,TOK_SITE_CHACL)) return -1;
  if (commands_add(_ctable,"site_change",do_site_change,do_site_help_change,TOK_SITE_CHANGE)) return -1;
  if (commands_add(_ctable,"site_changegrp",do_site_changegrp,do_site_help_changegrp,TOK_SITE_CHANGEGRP)) return -1;
  if (commands_add(_ctable,"site_checkperm",do_site_checkperm,NULL,TOK_SITE_CHECKPERM)) return -1;
  if (commands_add(_ctable,"site_chgrp",do_site_chgrp,NULL,TOK_SITE_CHGRP)) return -1;
  if (commands_add(_ctable,"site_chmod",do_site_chmod,NULL,TOK_SITE_CHMOD)) return -1;
  if (commands_add(_ctable,"site_chown",do_site_chown,NULL,TOK_SITE_CHOWN)) return -1;
  if (commands_add(_ctable,"site_chpass",do_site_chpass,NULL,TOK_SITE_CHPASS)) return -1;
  if (commands_add(_ctable,"site_chratio",do_site_chratio,do_site_help_chratio,TOK_SITE_CHRATIO)) return -1;
  if (commands_add(_ctable,"site_close",do_site,NULL,TOK_SITE_CLOSE)) return -1;
  if (commands_add(_ctable,"site_color",do_site_color,NULL,TOK_SITE_COLOR)) return -1;
  if (commands_add(_ctable,"site_delip",do_site_delip,do_site_help_delip,TOK_SITE_DELIP)) return -1;
  if (commands_add(_ctable,"site_deluser",do_site_deluser,do_site_help_deluser,TOK_SITE_DELUSER)) return -1;
  if (commands_add(_ctable,"site_flags",do_site_flags,NULL,TOK_SITE_FLAGS)) return -1;
  if (commands_add(_ctable,"site_free",do_site_free,NULL,TOK_SITE_FREE)) return -1;
  if (commands_add(_ctable,"site_ginfo",do_site_ginfo,NULL,TOK_SITE_GINFO)) return -1;
  if (commands_add(_ctable,"site_give",do_site_give,do_site_help_give,TOK_SITE_GIVE)) return -1;
  if (commands_add(_ctable,"site_group",do_site_group,do_site_help_group,TOK_SITE_GROUP)) return -1;
  if (commands_add(_ctable,"site_grpadd",do_site_grpadd,do_site_help_grpadd,TOK_SITE_GRPADD)) return -1;
  if (commands_add(_ctable,"site_grpaddip",do_site_grpaddip,do_site_help_grpaddip,TOK_SITE_GRPADDIP)) return -1;
  if (commands_add(_ctable,"site_grpchange",do_site_grpchange,do_site_help_grpchange,TOK_SITE_GRPCHANGE)) return -1;
  if (commands_add(_ctable,"site_grpdel",do_site_grpdel,do_site_help_grpdel,TOK_SITE_GRPDEL)) return -1;
  if (commands_add(_ctable,"site_grpdelip",do_site_grpdelip,do_site_help_grpdelip,TOK_SITE_GRPDELIP)) return -1;
  if (commands_add(_ctable,"site_grpkill",do_site_grpkill,NULL,TOK_SITE_GRPKILL)) return -1;
  if (commands_add(_ctable,"site_grpratio",do_site_grpratio,do_site_help_grpratio,TOK_SITE_GRPRATIO)) return -1;
  if (commands_add(_ctable,"site_grpren",do_site_grpren,do_site_help_grpren,TOK_SITE_GRPREN)) return -1;
  if (commands_add(_ctable,"site_gsinfo",do_site_gsinfo,NULL,TOK_SITE_GSINFO)) return -1;
  if (commands_add(_ctable,"site_help",do_site_help_command,NULL,TOK_SITE_HELP)) return -1;
  if (commands_add(_ctable,"site_idle",do_site_idle,NULL,TOK_SITE_IDLE)) return -1;
  if (commands_add(_ctable,"site_invite",do_site_invite,NULL,TOK_SITE_INVITE)) return -1;
  if (commands_add(_ctable,"site_kick",do_site_kick,NULL,TOK_SITE_KICK)) return -1;
  if (commands_add(_ctable,"site_kill",do_site_kill,NULL,TOK_SITE_KILL)) return -1;
  if (commands_add(_ctable,"site_killpath",do_site_killpath,NULL,TOK_SITE_KILLPATH)) return -1;
  if (commands_add(_ctable,"site_link",do_site_link,NULL,TOK_SITE_LINK)) return -1;
  if (commands_add(_ctable,"site_msg",do_site_msg,NULL,TOK_SITE_MSG)) return -1;
  if (commands_add(_ctable,"site_perm",do_site_perm,NULL,TOK_SITE_PERM)) return -1;
  if (commands_add(_ctable,"site_purge",do_site_purgeuser,NULL,TOK_SITE_PURGE)) return -1;
  if (commands_add(_ctable,"site_readd",do_site_readduser,do_site_help_readduser,TOK_SITE_READD)) return -1;
  if (commands_add(_ctable,"site_reload",do_site_reload,NULL,TOK_SITE_RELOAD)) return -1;
  if (commands_add(_ctable,"site_reopen",do_site,NULL,TOK_SITE_REOPEN)) return -1;
  if (commands_add(_ctable,"site_rusage",do_site_rusage,NULL,TOK_SITE_RUSAGE)) return -1;
  if (commands_add(_ctable,"site_savecfg",do_site_savecfg,NULL,TOK_SITE_SAVECFG)) return -1;
  if (commands_add(_ctable,"site_sections",do_site_sections,NULL,TOK_SITE_SECTIONS)) return -1;
  if (commands_add(_ctable,"site_showlog",do_site_showlog,NULL,TOK_SITE_SHOWLOG)) return -1;
  if (commands_add(_ctable,"site_shutdown",do_site,NULL,TOK_SITE_SHUTDOWN)) return -1;
  if (commands_add(_ctable,"site_su",do_site_su,do_site_help_su,TOK_SITE_SU)) return -1;
  if (commands_add(_ctable,"site_tagline",do_site_tagline,NULL,TOK_SITE_TAGLINE)) return -1;
  if (commands_add(_ctable,"site_take",do_site_take,do_site_help_take,TOK_SITE_TAKE)) return -1;
#ifdef DEBUG
  if (commands_add(_ctable,"site_test",do_site_test,NULL,TOK_SITE_TEST)) return -1;
#endif
  if (commands_add(_ctable,"site_unlock",do_site_unlock,NULL,TOK_SITE_UNLOCK)) return -1;
  if (commands_add(_ctable,"site_uptime",do_site,NULL,TOK_SITE_UPTIME)) return -1;
  if (commands_add(_ctable,"site_user",do_site_user,NULL,TOK_SITE_USER)) return -1;
  if (commands_add(_ctable,"site_utime",do_site_utime,NULL,TOK_SITE_UTIME)) return -1;
  if (commands_add(_ctable,"site_vars",do_site_vars,NULL,TOK_SITE_VARS)) return -1;
  if (commands_add(_ctable,"site_vars_group",do_site_vars_group,NULL,TOK_SITE_VARS_GROUP)) return -1;
  if (commands_add(_ctable,"site_vars_user",do_site_vars_user,NULL,TOK_SITE_VARS_USER)) return -1;
  if (commands_add(_ctable,"site_version",do_site_version,NULL,TOK_SITE_VERSION)) return -1;
  if (commands_add(_ctable,"site_vfsadd",do_site_vfsadd,NULL,TOK_SITE_VFSADD)) return -1;
  if (commands_add(_ctable,"site_vfsdel",do_site_vfsdel,NULL,TOK_SITE_VFSDEL)) return -1;
  if (commands_add(_ctable,"site_who",do_site,NULL,TOK_SITE_WHO)) return -1;
  if (commands_add(_ctable,"site_wipe",do_site_wipe,NULL,TOK_SITE_WIPE)) return -1;

  return 0;
}

/** \brief Search for command in registered commands
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] str Command name to find
 *
 * \return
 * - a wzd_command_t structure if the command has been found
 * - NULL if not found
 */
wzd_command_t * commands_find(CHTBL * _ctable, wzd_string_t *str)
{
  wzd_command_t * command = NULL;

  if (!_ctable || !str) return NULL;

  str_tolower(str);

  chtbl_lookup(_ctable, str_tochar(str), (void**)&command);

  return command;
}

/** \brief Set permissions associated to a command
 *
 * Replace permissions for the specified command.
 * The command must exist.
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] permname The permission name
 * \param[in] permline A string describing permissions
 * \return 0 if command is ok
 */
int commands_set_permission(CHTBL * _ctable, const char * permname, const char * permline)
{
  wzd_command_t * command;
  wzd_command_perm_t * old_perms;
  wzd_string_t * str = STR(permname);

  command = commands_find(_ctable,str);
  str_deallocate(str);
  if (!command) return -1;

  old_perms = command->perms;
  command->perms = NULL;

  if ( ! perm_add_perm(permname, permline, &command->perms) ) {
    perm_free_recursive(old_perms);
    return 0;
  } else {
    perm_free_recursive(command->perms);
    command->perms = old_perms;
    return 1;
  }
}

/** \brief Add permissions to a command
 *
 * Add permissions for the specified command.
 * The command must exist.
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] permname The permission name
 * \param[in] permline A string describing permissions, to be appended
 * \return 0 if command is ok
 */
int commands_add_permission(CHTBL * _ctable, const char * permname, const char * permline)
{
  wzd_command_t * command;
  wzd_string_t * str = STR(permname);

  command = commands_find(_ctable,str);
  str_deallocate(str);
  if (!command) return -1;

  return perm_add_perm(permname, permline, &command->perms);
}

/** \brief Check if user is authorized to run specified command
 *
 * Check if the user in the specific context is allowed to run the command.
 *
 * \param[in] command The command name
 * \param[in] context The client context
 * \return 0 if ok
 */
int commands_check_permission(wzd_command_t * command, wzd_context_t * context)
{
  if (!command) return 0;

  return perm_check_perm(command->perms, context);
}

/** \brief Delete permissions associated to a command
 *
 * Delete permissions associated to the command.
 *
 * \param[in] _ctable Hash table containing commands
 * \param[in] str The command name
 * \return 0 if ok
 */
int commands_delete_permission(CHTBL * _ctable, wzd_string_t * str)
{
  wzd_command_t * command;

  command = commands_find(_ctable,str);
  if (!command) return 1;
  
  perm_free_recursive(command->perms);
  command->perms = NULL;

  return 0;
}

