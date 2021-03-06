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
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dlfcn.h>
#endif

#if defined(BSD) && !(defined(__MACH__) && defined(__APPLE__))
#define DL_ARG  DL_LAZY
#else
#define DL_ARG  RTLD_NOW
#endif

#include "wzd_structs.h"

#include "wzd_cache.h"
#include "wzd_fs.h"
#include "wzd_log.h"
#include "wzd_misc.h"
#include "wzd_messages.h"
#include "wzd_mod.h"
#include "wzd_user.h"

#include "wzd_debug.h"

#ifdef NEED_UNDERSCORE
#define DL_PREFIX "_"
#else
#define DL_PREFIX
#endif

#endif /* WZD_USE_PCH */

struct event_entry_t {
  unsigned long mask;
  const char *name;
};

struct event_entry_t event_tab[] = {
  { EVENT_LOGIN, "LOGIN" },
  { EVENT_LOGOUT, "LOGOUT" },
  { EVENT_PREUPLOAD, "PREUPLOAD" },
  { EVENT_POSTUPLOAD, "POSTUPLOAD" },
  { EVENT_POSTUPLOAD_DENIED, "POSTUPLOAD_DENIED" },
  { EVENT_PREDOWNLOAD, "PREDOWNLOAD" },
  { EVENT_POSTDOWNLOAD, "POSTDOWNLOAD" },
  { EVENT_PREMKDIR, "PREMKDIR" },
  { EVENT_MKDIR, "MKDIR" },
  { EVENT_RMDIR, "RMDIR" },
  { EVENT_DELE, "DELE" },
  { EVENT_PRERENAME, "PRERENAME" },
  { EVENT_POSTRENAME, "POSTRENAME" },
  { EVENT_SITE, "SITE" },
  { EVENT_WIPE, "WIPE" },
  { EVENT_PREWIPE, "PREWIPE" },
  { 0, NULL },
};

extern void _cleanup_shell_command(char * buffer, size_t length);

static protocol_handler_t * proto_handler_list=NULL;
static unsigned int _reply_code;

int hook_add_protocol(const char *signature, unsigned int sig_len, fcn_handler handler)
{
  protocol_handler_t * proto;

  if (!signature || !handler || sig_len==0) return -1;

  proto = wzd_malloc (sizeof(protocol_handler_t));
  proto->sig = wzd_malloc(sig_len+1);
  memcpy(proto->sig,signature,sig_len);
  proto->sig[sig_len] = '\0';
  proto->siglen = sig_len;
  proto->handler = handler;
  proto->next_proto = proto_handler_list;

  proto_handler_list = proto;

  return 0;
}

void hook_free_protocols(void)
{
  protocol_handler_t * proto, * next_proto;
  proto = proto_handler_list;

  while (proto)
  {
    next_proto = proto->next_proto;
    if (proto->sig) wzd_free(proto->sig);
    wzd_free(proto);
    proto = next_proto;
  }
  proto_handler_list = NULL;
}

protocol_handler_t * hook_check_protocol(const char *str)
{
  protocol_handler_t * proto;

  proto = proto_handler_list;
  while (proto)
  {
    if (strncmp(str,proto->sig,proto->siglen)==0)
      return proto;
    proto = proto->next_proto;
  }

  return NULL;
}



/** free hook list */
void hook_free(wzd_hook_t **hook_list)
{
  wzd_hook_t * current_hook, * next_hook;

  current_hook = *hook_list;

  while (current_hook) {
    next_hook = current_hook->next_hook;

    if (current_hook->external_command)
      free(current_hook->external_command);
    if (current_hook->opt) free(current_hook->opt);
#ifdef DEBUG
    current_hook->mask = 0;
    current_hook->hook = NULL;
    current_hook->external_command = NULL;
    current_hook->opt=NULL;
    current_hook->next_hook = NULL;
#endif /* DEBUG */
    free(current_hook);

    current_hook = next_hook;
  }

  *hook_list = NULL;
}

/** register a new hook */
int hook_add(wzd_hook_t ** hook_list, unsigned long mask, void_fct hook)
{
  wzd_hook_t * current_hook, * new_hook;

  new_hook = malloc(sizeof(wzd_hook_t));
  if (!new_hook) return 1;

  new_hook->mask = mask;
  new_hook->hook = hook;
  new_hook->opt = NULL;
  new_hook->external_command = NULL;
  new_hook->next_hook = NULL;

  current_hook = *hook_list;

  if (!current_hook) {
    *hook_list = new_hook;
    return 0;
  }

  while (current_hook->next_hook) {
    current_hook = current_hook->next_hook;
  }

  current_hook->next_hook = new_hook;

  return 0;
}

int hook_add_external(wzd_hook_t ** hook_list, unsigned long mask, const char *command)
{
  wzd_hook_t * current_hook, * new_hook;

  new_hook = malloc(sizeof(wzd_hook_t));
  if (!new_hook) return 1;

  new_hook->mask = mask;
  new_hook->hook = NULL;
  new_hook->opt = NULL;
  new_hook->external_command = strdup(command);
  new_hook->next_hook = NULL;

  current_hook = *hook_list;

  if (!current_hook) {
    *hook_list = new_hook;
    return 0;
  }

  while (current_hook->next_hook) {
    current_hook = current_hook->next_hook;
  }

  current_hook->next_hook = new_hook;

  return 0;
}

/** remove hook from list */
int hook_remove(wzd_hook_t **hook_list, unsigned long mask, void_fct hook)
{
  wzd_hook_t * current_hook, * previous_hook=NULL;

  if (!hook_list || !hook) return 1;

  current_hook = *hook_list;

  while (current_hook)
  {
    if (current_hook->mask == mask && current_hook->hook == hook)
    {
      if (previous_hook)
        previous_hook->next_hook = current_hook->next_hook;
      else
        *hook_list = current_hook->next_hook;

      if (current_hook->external_command)
        free(current_hook->external_command);
      if (current_hook->opt) free(current_hook->opt);
#ifdef DEBUG
      current_hook->mask = 0;
      current_hook->hook = NULL;
      current_hook->external_command = NULL;
      current_hook->opt=NULL;
      current_hook->next_hook = NULL;
#endif /* DEBUG */
      free(current_hook);

      return 0;
    }
    previous_hook = current_hook;
    current_hook = current_hook->next_hook;
  }

  return 1; /* not found */
}

/** hook_call_external: events */
int hook_call_external(wzd_hook_t *hook, unsigned int code)
{
  char buffer[1024];
  char *buffer_args;
  FILE *command_output;
  size_t l_command;
  protocol_handler_t * proto;

  if (!hook || !hook->external_command) return 1;
  l_command = strlen(hook->external_command);
  if (l_command>=1022) return 1;
  /* replace cookies in args */
  {
    wzd_context_t * context = GetMyContext();
    wzd_user_t * user = context ? GetUserByID(context->userid) : NULL;
    wzd_group_t * group = context ? GetGroupByID(user->groups[0]) : NULL;

    cookie_parse_buffer(hook->external_command,user,group,context,buffer,sizeof(buffer));
  }
  l_command = strlen(buffer);
  while (l_command>0 && (buffer[l_command-1]=='\n' || buffer[l_command-1]=='\r'))
    buffer[--l_command] = '\0';
  _reply_code = code;
  /* we can use protocol hooks here */
  proto = hook_check_protocol(buffer);
  if (proto)
  {
    /* we need to reformat args */
    if ( *(buffer+proto->siglen) == '"' ) { /* search matching " */
      buffer_args = strchr(buffer+proto->siglen+1,'"');
      *buffer_args++ = '\0'; /* eat trailing " */
      if (*buffer_args==' ') *buffer_args++ = '\0';
      return (*proto->handler)(buffer+proto->siglen+1,buffer_args);
    } else
      buffer_args = strchr(buffer+proto->siglen,' ');
    if (buffer_args) {
      *buffer_args++ = '\0';
    } else {
      buffer_args = NULL;
    }
    return (*proto->handler)(buffer+proto->siglen,buffer_args);
  }
  else
  {
/*    *(buffer+l_command++) = ' ';*/
    /* SECURITY filter buffer for shell special characters ! */
    _cleanup_shell_command(buffer,sizeof(buffer));
    if ( (command_output = popen(buffer,"r")) == NULL ) {
      out_log(LEVEL_HIGH,"Hook '%s': unable to popen\n",hook->external_command);
      out_log(LEVEL_INFO,"Failed command: '%s'\n",buffer);
      return 1;
    }
    while (fgets(buffer,1023,command_output) != NULL)
    {
      out_log(LEVEL_INFO,"hook: %s\n",buffer);
    }
    return pclose(command_output);
  }

  return 0;
}

char * event2str(const unsigned long mask)
{
  int i=0;

  while (event_tab[i].mask != 0)
  {
    if (event_tab[i].mask == mask) return (char*)event_tab[i].name;
    i++;
  }
  return 0;
}

unsigned long str2event(const char *s)
{
  int i=0;

  while (event_tab[i].mask != 0)
  {
    if (strcasecmp(s,event_tab[i].name)==0) return event_tab[i].mask;
    i++;
  }
  return 0;
}



/** check a module file */
int module_check(const char *filename)
{
  char path[1024];
  void * handle;
  void * ptr;
  char * error;
  fs_filestat_t st;
  int ret;
  int mode = DL_ARG;

  if (!filename || filename[0]=='\0') return -1;
#ifndef _MSC_VER
  if (filename[0] == '/')
#else
  if (filename[0] == '/' || filename[1] == ':')
#endif
    strncpy(path,filename,1023);
  else
  { /* relative path */
    if (strlen(filename) >= 1022) return -1;
    path[0] = '.';
    path[1] = '/';
    strcpy(path+2,filename);
  }

  ret = fs_file_lstat(path,&st);
  if (ret) {
    out_err(LEVEL_HIGH,"Could not stat module '%s'\n",filename);
    out_err(LEVEL_HIGH,"errno: %d error: %s\n",errno, strerror(errno));
    return -1;
  }

#ifdef RTLD_GLOBAL
  mode |= RTLD_GLOBAL;
#else
  out_log(LEVEL_NORMAL,"WARNING Can't make loaded symbols global on this platform while loading %s\n",path);
#endif
  /* test dlopen */
  handle = dlopen(path,mode);
  if (!handle) {
    out_err(LEVEL_HIGH,"Could not dlopen module '%s'\n",filename);
    out_err(LEVEL_HIGH," ^-- Reason: %s\n",dlerror());
    return 1;
  }

  /* check basic functions */
  ptr = dlsym(handle,DL_PREFIX STR_MODULE_INIT);
#ifndef _MSC_VER
  if ((error = dlerror()) != NULL)
#else
  error = "";
  if ( !ptr )
#endif
  {
    out_err(LEVEL_HIGH,"Unable to find function WZD_MODULE_INIT in module %s\n%s\n",filename,error);
    dlclose(handle);
    return 1;
  }

/*
  ptr = dlsym(handle,DL_PREFIX "hook_table");
  if ((error = dlerror()) != NULL) {
    out_log(LEVEL_HIGH,"Unable to find structure 'hook_table' in module %s\n%s\n",filename,error);
    dlclose(handle);
    return 1;
  }

  {
    typedef void (*myfct)(void);
    myfct f;
    f = (myfct)dlsym(handle,DL_PREFIX "moduletest");
    out_err(LEVEL_HIGH,"main prog mainConfig: %lx\n",(unsigned long)getlib_mainConfig()->logfile);
    if (f)
      f();
    else
      out_err(LEVEL_HIGH,"Could not find moduletest\n");
  }
*/

  dlclose(handle);
  return 0;
}

/** add a module to the list */
int module_add(wzd_module_t ** module_list, const char *name)
{
  wzd_module_t * current_module, * new_module;

  new_module = malloc(sizeof(wzd_module_t));
  if (!new_module) return 1;

  new_module->name = strdup(name);
  new_module->handle = NULL;
  new_module->next_module = NULL;

  current_module = *module_list;

  if (!current_module) {
    *module_list = new_module;
    return 0;
  }

  while (current_module->next_module) {
    current_module = current_module->next_module;
  }

  current_module->next_module = new_module;

  return 0;
}

/** load a module - module really should have been checked before ! */
int module_load(wzd_module_t *module)
{
  char path[1024];
  void * handle;
  int ret;
  char * filename;
  fcn_module_init f_init;
  int mode = DL_ARG;
#ifdef DEBUG
  char * error;
#endif

  filename = module->name;

#ifndef _MSC_VER
  if (filename[0] == '/')
#else
  if (filename[0] == '/' || filename[1] == ':')
#endif
    strncpy(path,filename,1023);
  else
  { /* relative path */
    if (strlen(filename) >= 1022) return -1;
    path[0] = '.';
    path[1] = '/';
    strcpy(path+2,filename);
  }

#ifdef RTLD_GLOBAL
  mode |= RTLD_GLOBAL;
#else
  out_log(LEVEL_NORMAL,"WARNING Can't make loaded symbols global on this platform while loading %s\n",path);
#endif
  /* test dlopen */
  handle = dlopen(path,mode);
  if (!handle) return -1;

  f_init = (fcn_module_init)dlsym(handle,DL_PREFIX STR_MODULE_INIT);
#ifdef DEBUG
#ifndef _MSC_VER
  if ((error = dlerror()) != NULL)
#else
  error = "";
  if ( !f_init )
#endif
  {
    out_log(LEVEL_CRITICAL,"Unable to find function WZD_MODULE_INIT in module %s\n%s\n",filename,error);
    out_log(LEVEL_CRITICAL,"THIS SHOULD HAVE BEEN CHECKED BEFORE !\n");
    dlclose(handle);
    return 1;
  }
#endif

  ret = (f_init)();
  if (ret) {
    out_log(LEVEL_HIGH,"ERROR could not load module %s\n",module->name);
    dlclose(handle);
    return ret;
  }

  module->handle = handle;

#ifdef WZD_DBG_MODULES
  out_log(LEVEL_INFO,"MODULE: loaded '%s' at address %p\n",filename,handle);
#endif

  return ret;
}

/** unload module, and remove it from list */
int module_unload(wzd_module_t **module_list, const char *name)
{
  wzd_module_t * current_module, * previous_module=NULL;
  fcn_module_close f_close;

  current_module = *module_list;
  if (!current_module || !name) return 1; /* not found */

  while (current_module) {

    if (strcmp(current_module->name,name)==0)
    {
#ifdef WZD_DBG_MODULES
      out_log(LEVEL_INFO,"MODULE: unloading '%s' at address %p\n",current_module->name,current_module->handle);
#endif
      f_close = (fcn_module_close)dlsym(current_module->handle,DL_PREFIX STR_MODULE_CLOSE);
      if (f_close) (*f_close)();

/** \todo XXX FIXME
 * dlclose() on a shared lib in a multithread application will likely cause a segfault
 * when thread exits, because thread will try to free some specific thread-vars
 * update: it seems behaviour improves with linux 2.6.5
 */
/*      dlclose(current_module->handle);*/

      if (previous_module)
        previous_module->next_module = current_module->next_module;
      else
        *module_list = current_module->next_module;

      if (current_module->name)
        free(current_module->name);
#ifdef DEBUG
      current_module->handle = NULL;
      current_module->name = NULL;
      current_module->next_module = NULL;
#endif /* DEBUG */
      free(current_module);
      return 0;
    }

    previous_module = current_module;
    current_module = current_module->next_module;
  }
  return 1; /* not found */
}

/** free module list */
void module_free(wzd_module_t ** module_list)
{
  wzd_module_t * current_module, * next_module;
  fcn_module_close f_close;

  current_module = *module_list;

  while (current_module) {
    next_module = current_module->next_module;

#ifdef WZD_DBG_MODULES
    out_log(LEVEL_INFO,"MODULE: unloading '%s' at address %p\n",current_module->name,current_module->handle);
#endif
    if (current_module->handle) {
      f_close = (fcn_module_close)dlsym(current_module->handle,DL_PREFIX STR_MODULE_CLOSE);
      if (f_close) (*f_close)();

      dlclose(current_module->handle);
    }

    if (current_module->name)
      free(current_module->name);
#ifdef DEBUG
    current_module->handle = NULL;
    current_module->name = NULL;
    current_module->next_module = NULL;
#endif /* DEBUG */
    free(current_module);

    current_module = next_module;
  }

  *module_list = NULL;
}

/** \brief Get module name */
const char * module_get_name(wzd_module_t * module)
{
  char ** module_name;

  if (module->handle)
    module_name = (char**)dlsym(module->handle,DL_PREFIX "module_name");
  else
    return NULL;

  return (module_name) ? *module_name : NULL;
}

/** \brief Get module version */
const char * module_get_version(wzd_module_t * module)
{
  char ** module_version;

  if (module->handle)
    module_version = (char**)dlsym(module->handle,DL_PREFIX "module_version");
  else
    return NULL;

  return (module_version) ? *module_version : NULL;
}

unsigned int hook_get_current_reply_code(void)
{
  return _reply_code;
}


