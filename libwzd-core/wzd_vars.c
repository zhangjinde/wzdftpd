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

#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	/* struct in_addr (wzd_misc.h) */
#endif

#include <sys/stat.h>

#include "wzd_structs.h"

#include "wzd_libmain.h"
#include "wzd_misc.h"

#include "wzd_configfile.h"
#include "wzd_fs.h"
#include "wzd_group.h"
#include "wzd_vars.h"
#include "wzd_log.h"
#include "wzd_mutex.h"
#include "wzd_user.h"


#include "wzd_debug.h"

#endif /* WZD_USE_PCH */



static struct wzd_shm_vars_t * _shm_vars[32] = { NULL };



int vars_get(const char *varname, char *data, size_t datalength, wzd_config_t * config)
{
  if (!config) return 1;

  if (strcasecmp(varname,"bw")==0) {
    snprintf(data,datalength,"%lu",get_bandwidth(NULL,NULL));
    return 0;
  }
  if (strcmp(varname,"loglevel")==0) {
    const char * str;

    str = config_get_value(config->cfg_file, "GLOBAL", "loglevel");
    if (str) {
      snprintf(data,datalength,"%s",str);
      return 0;
    }
    snprintf(data,datalength,"%s",loglevel2str(config->loglevel));
    return 0;
  }
  if (strcasecmp(varname,"max_users")==0) {
    snprintf(data,datalength,"%u",config->max_users);
    return 0;
  }
  if (strcasecmp(varname,"max_dl")==0) {
    snprintf(data,datalength,"%u",config->global_dl_limiter.maxspeed);
    return 0;
  }
  if (strcasecmp(varname,"max_threads")==0) {
    snprintf(data,datalength,"%d",config->max_threads);
    return 0;
  }
  if (strcasecmp(varname,"max_ul")==0) {
    snprintf(data,datalength,"%u",config->global_ul_limiter.maxspeed);
    return 0;
  }
  if (strcasecmp(varname,"pasv_low")==0) {
    snprintf(data,datalength,"%u",config->pasv_low_range);
    return 0;
  }
  if (strcasecmp(varname,"pasv_high")==0) {
    snprintf(data,datalength,"%u",config->pasv_high_range);
    return 0;
  }
  if (strcmp(varname,"port")==0) {
    const char * str;

    str = config_get_value(config->cfg_file, "GLOBAL", "port");
    if (str) {
      snprintf(data,datalength,"%s",str);
      return 0;
    }
    snprintf(data,datalength,"%u",config->port);
    return 0;
  }
  if (strcmp(varname,"uptime")==0) {
    time_t t;

    (void)time(&t);
    t = t - config->server_start;
    snprintf(data,datalength,"%lu",(unsigned long)t);
    return 0;
  }

  return 1;
}

/** \brief Change value of server variable
 *
 * \todo we should change the value in config->cfg_file
 */
int vars_set(const char *varname, const char *data, size_t datalength, wzd_config_t * config)
{
  int i;
  unsigned long ul;
  char *ptr;

  if (!data || !config) return 1;

  if (strcasecmp(varname,"deny_access_files_uploaded")==0) {
    ul = strtoul(data,NULL,0);
    if (ul==1) { CFG_SET_OPTION(config,CFG_OPT_DENY_ACCESS_FILES_UPLOADED); return 0; }
    if (ul==0) { CFG_CLR_OPTION(config,CFG_OPT_DENY_ACCESS_FILES_UPLOADED); return 0; }
    return 1;
  }
  if (strcasecmp(varname,"hide_dotted_files")==0) {
    ul = strtoul(data,NULL,0);
    if (ul==1) { CFG_SET_OPTION(config,CFG_OPT_HIDE_DOTTED_FILES); return 0; }
    if (ul==0) { CFG_CLR_OPTION(config,CFG_OPT_HIDE_DOTTED_FILES); return 0; }
    return 1;
  }
  if (strcasecmp(varname,"loglevel")==0) {
    i = str2loglevel(data);
    if (i==-1) {
      return 1;
    }
    config->loglevel = i;
    return 0;
  }
  if (strcasecmp(varname,"max_users")==0) {
    ul = strtoul(data,&ptr,0);
    if (ptr && *ptr == '\0') {
      config->max_users = ul;
      return 0;
    }
  }
  if (strcasecmp(varname,"max_dl")==0) {
    ul = strtoul(data,&ptr,0);
    if (ptr && *ptr == '\0') {
      config->global_dl_limiter.maxspeed = ul;
      return 0;
    }
  }
  if (strcasecmp(varname,"max_threads")==0) {
    ul = strtoul(data,&ptr,0);
    if (ptr && *ptr == '\0') {
      config->max_threads = ul;
      return 0;
    }
  }
  if (strcasecmp(varname,"max_ul")==0) {
    ul = strtoul(data,&ptr,0);
    if (ptr && *ptr == '\0') {
      config->global_ul_limiter.maxspeed = ul;
      return 0;
    }
  }
  if (strcasecmp(varname,"pasv_low")==0) {
    ul = strtoul(data,NULL,0);
    if (ul < 65535 && ul < config->pasv_high_range) {
      config->pasv_low_range = ul;
      return 0;
    }
  }
  if (strcasecmp(varname,"pasv_high")==0) {
    ul = strtoul(data,NULL,0);
    if (ul < 65535 && ul > config->pasv_low_range) {
      config->pasv_high_range = ul;
      return 0;
    }
  }

  return 1;
}

int vars_user_get(const char *username, const char *varname, char *data, size_t datalength, wzd_config_t * config)
{
  wzd_user_t * user;
  wzd_group_t * group;

  if (!username || !varname) return 1;

  user = GetUserByName(username);
  if (!user) return 1;

  if (strcasecmp(varname,"group")==0) {
    if (user->group_num > 0) {
      group = GetGroupByID(user->groups[0]);
      snprintf(data,datalength,"%s",group->groupname);
    } else
      snprintf(data,datalength,"no group");
    return 0;
  }
  if (strcasecmp(varname,"home")==0) {
    snprintf(data,datalength,"%s",user->rootpath);
    return 0;
  }
  if (strcasecmp(varname,"max_dl")==0) {
    snprintf(data,datalength,"%u",user->max_dl_speed);
    return 0;
  }
  if (strcasecmp(varname,"max_ul")==0) {
    snprintf(data,datalength,"%u",user->max_ul_speed);
    return 0;
  }
  if (strcasecmp(varname,"credits")==0) {
    snprintf(data,datalength,"%" PRIu64,user->credits);
    return 0;
  }
  if (strcasecmp(varname,"name")==0) {
    snprintf(data,datalength,"%s",user->username);
    return 0;
  }
  if (strcasecmp(varname,"tag")==0) {
    if (user->tagline[0] != '\0')
      snprintf(data,datalength,"%s",user->tagline);
    else
      snprintf(data,datalength,"no tagline set");
    return 0;
  }

  return 1;
}

int vars_user_addip(const char *username, const char *ip, wzd_config_t *config)
{
  wzd_user_t *user;
  int ret;

  if (!username || !ip) return 1;

  user = GetUserByName(username);
  if (!user) return -1;

  do {

    ret = ip_inlist(user->ip_list, ip);
    if (ret) return 1; /* already present */

    ret = ip_add_check(&user->ip_list, ip, 1 /* is_allowed */);

/*    ip = strtok_r(NULL," \t\r\n",&ptr);*/
    ip = NULL; /** \todo add only one ip (for the moment) */
  } while (ip);

  /* commit to backend */
  return backend_mod_user(config->backends->filename, user->uid, user, _USER_IP);
}

int vars_user_delip(const char *username, const char *ip, wzd_config_t *config)
{
  char *ptr_ul;
  wzd_user_t *user;
  unsigned long ul;
  int ret;

  if (!username || !ip) return 1;

  user = GetUserByName(username);
  if (!user) return -1;

  do {

    /* try to take argument as a slot number */
    ul = strtoul(ip,&ptr_ul,0);
    if (*ptr_ul=='\0') {
      unsigned int i;
      struct wzd_ip_list_t * current_ip;

      current_ip = user->ip_list;
      for (i=1; i<ul && current_ip != NULL; i++) {
        current_ip = current_ip->next_ip;
      }
      if (current_ip == NULL) return 2; /* not found */
      ret = ip_remove(&user->ip_list,current_ip->regexp);
      if (ret != 0) return -1;
    } else { /* if (*ptr=='\0') */

      ret = ip_remove(&user->ip_list,ip);
      if (ret != 0) return 3;

    } /* if (*ptr=='\0') */

/*    ip = strtok_r(NULL," \t\r\n",&ptr);*/
    ip = NULL; /** \todo add only one ip (for the moment) */
  } while (ip);

  /* commit to backend */
  return backend_mod_user(config->backends->filename, user->uid, user, _USER_IP);
}

int vars_user_set(const char *username, const char *varname, const char *data, size_t datalength, wzd_config_t * config)
{
  wzd_user_t * user;
  unsigned long mod_type;
  unsigned long ul;
  u64_t ull;
  char *ptr = 0;
  int ret;

  if (!username || !varname) return 1;

  user = GetUserByName(username);
  if (!user) return -1;

  /* find modification type */
  mod_type = _USER_NOTHING;

  /* addip */
  if (strcmp(varname, "addip")==0) {
    return vars_user_addip(username, data, config);
  }
  /* credits */
  else if (strcmp(varname, "credits")==0) {
    if (*data < '0' || *data > '9') /* invalid number */
      return -1;
    ull = strtoull(data, &ptr, 0);
    if (*ptr || ptr == data) /* invalid number */
      return -1;
    user->credits = ull;
    mod_type = _USER_CREDITS;
  }
  /* bytes_ul and bytes_dl should never be changed ... */
  /* delip */
  else if (strcmp(varname, "delip")==0) {
    return vars_user_delip(username, data, config);
  }
  /* flags */ /* TODO accept modifications style +f or -f */
  else if (strcmp(varname, "flags")==0) {
    strncpy(user->flags, data, MAX_FLAGS_NUM-1);
    mod_type = _USER_FLAGS;
  }
  /* homedir */
  else if (strcmp(varname, "home")==0) {
    /* check if homedir exist */
    {
      fs_filestat_t s;
      if (fs_file_stat(data,&s) || !S_ISDIR(s.mode)) {
        /* Homedir does not exist */
        return 1;
      }
    }
    mod_type = _USER_ROOTPATH;
    strncpy(user->rootpath, data, WZD_MAX_PATH);
  }
  /* leech_slots */
  else if (strcmp(varname, "leech_slots")==0) {
    ul=strtoul(data, &ptr, 0);
    /* TODO compare with USHORT_MAX */
    if (*ptr) return -1;
    mod_type = _USER_LEECHSLOTS;
    user->leech_slots = (unsigned short)ul;
  }
  /* max_dl */
  else if (strcmp(varname, "max_dl")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_MAX_DLS;
    user->max_dl_speed = ul;
  }
  /* max_idle */
  else if (strcmp(varname, "max_idle")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_IDLE;
    user->max_idle_time = ul;
  }
  /* max_ul */
  else if (strcmp(varname, "max_ul")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_MAX_ULS;
    user->max_ul_speed = ul;
  }
  /* num_logins */
  else if (strcmp(varname, "num_logins")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_NUMLOGINS;
    user->num_logins = (unsigned short)ul;
  }
  else if (strcmp(varname, "logins_per_ip)")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_LOGINSPERIP;
    user->logins_per_ip = (unsigned short)ul;
  }
  /* pass */
  else if (strcmp(varname, "pass")==0) {
    mod_type = _USER_USERPASS;
    strncpy(user->userpass, data, sizeof(user->userpass));
  }
  /* perms */
  else if (strcmp(varname, "perms")==0) {
    ul=strtoul(data, &ptr, 0);
    if (*ptr) return -1;
    mod_type = _USER_PERMS;
    user->userperms = ul;
  }
  /* ratio */
  else if (strcmp(varname, "ratio")==0) {
    ul=strtoul(data, &ptr,0);
    if (*ptr) return -1;
    mod_type = _USER_RATIO;
    user->ratio = ul;
  }
  /* tagline */
  else if (strcmp(varname, "tag")==0) {
    mod_type = _USER_TAGLINE;
    strncpy(user->tagline, data, sizeof(user->tagline));
  }
  /* uid */ /* FIXME useless ? */
  /* username (?) */
  else if (strcmp(varname, "name")==0) {
    mod_type = _USER_USERNAME;
    strncpy(user->username, data, sizeof(user->username));
  }
  /* creator */
  else if (strcmp(varname, "creator")==0) {
    wzd_user_t * creator;
    creator = GetUserByName(data);
    if (creator) {
      mod_type = _USER_CREATOR;
      user->creator = creator->uid;
    } else return -1;
  }
  /* user_slots */
  else if (strcmp(varname, "user_slots")==0) {
    ul=strtoul(data, &ptr, 0);
    /* TODO compare with USHORT_MAX */
    if (*ptr) return -1;
    mod_type = _USER_USERSLOTS;
    user->user_slots = (unsigned short)ul;
  }

  /* commit to backend */
  ret = backend_mod_user(config->backends->filename, user->uid, user, mod_type);

  return ret;
}

int vars_user_new(const char *username, const char *pass, const char *groupname, wzd_config_t * config)
{
  wzd_user_t * newuser;
  int err;

  if (!username || !groupname || !config) return -1;

  newuser = user_create(username,pass,groupname,NULL,config,&err);
  if (newuser == NULL) return err;

  /* add it to backend */
  err = backend_mod_user(config->backends->filename,0,newuser,_USER_CREATE);

  if (err) { /* problem adding user */
    user_free(newuser);
  }

  return err ? 1 : 0;
}

int vars_group_get(const char *groupname, const char *varname, char *data, size_t datalength, wzd_config_t * config)
{
  wzd_group_t * group;

  if (!groupname || !varname) return 1;

  group = GetGroupByName(groupname);
  if (!group) return 1;

  if (strcasecmp(varname,"home")==0) {
    snprintf(data,datalength,"%s",group->defaultpath);
    return 0;
  }
  if (strcasecmp(varname,"max_dl")==0) {
    snprintf(data,datalength,"%u",group->max_dl_speed);
    return 0;
  }
  if (strcasecmp(varname,"max_ul")==0) {
    snprintf(data,datalength,"%u",group->max_ul_speed);
    return 0;
  }
  if (strcasecmp(varname,"name")==0) {
    snprintf(data,datalength,"%s",group->groupname);
    return 0;
  }
  if (strcasecmp(varname,"tag")==0) {
    if (group->tagline[0] != '\0')
      snprintf(data,datalength,"%s",group->tagline);
    else
      snprintf(data,datalength,"no tagline set");
    return 0;
  }

  return 1;
}

int vars_group_set(const char *groupname, const char *varname, const char *data, size_t datalength, wzd_config_t * config)
{
  wzd_group_t * group;
  unsigned long mod_type;
  unsigned long ul;
  char *ptr;
  int ret;

  if (!groupname || !varname) return 1;

  group = GetGroupByName(groupname);
  if (!group) return -1;

  /* find modification type */
  mod_type = _GROUP_NOTHING;

  /* groupname */
  if (strcmp(varname,"name")==0) {
    mod_type = _GROUP_GROUPNAME;
    strncpy(group->groupname,data,sizeof(group->groupname));
    /* NOTE: we do not need to iterate through users, group is referenced
     * by id, not by name
     */
  }
  /* tagline */
  else if (strcmp(varname,"tag")==0) {
    mod_type = _GROUP_TAGLINE;
    strncpy(group->tagline,data,sizeof(group->tagline));
  }
  /* homedir */
  else if (strcmp(varname,"home")==0) {
    /* check if homedir exist */
    {
      fs_filestat_t s;
      if (fs_file_stat(data,&s) || !S_ISDIR(s.mode)) {
        /* Homedir does not exist */
        return 2;
      }
    }
    mod_type = _GROUP_DEFAULTPATH;
    strncpy(group->defaultpath,data,WZD_MAX_PATH);
  }
  /* max_idle */
  else if (strcmp(varname,"max_idle")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) { mod_type = _GROUP_IDLE; group->max_idle_time = ul; }
  }
  /* perms */
  else if (strcmp(varname,"perms")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) { mod_type = _GROUP_GROUPPERMS; group->groupperms = ul; }
  }
  /* flags */ /* TODO accept modifications style +f or -f */
  else if (strcmp(varname, "flags")==0) {
    strncpy(group->flags, data, MAX_FLAGS_NUM-1);
    mod_type = _GROUP_FLAGS;
  }
  /* max_ul */
  else if (strcmp(varname,"max_ul")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) { mod_type = _GROUP_MAX_ULS; group->max_ul_speed = ul; }
  }
  /* max_dl */
  else if (strcmp(varname,"max_dl")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) { mod_type = _GROUP_MAX_DLS; group->max_dl_speed = ul; }
  }
  /* num_logins */
  else if (strcmp(varname,"num_logins")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) { mod_type = _GROUP_NUMLOGINS; group->num_logins = (unsigned short)ul; }
  }
  /* ratio */
  else if (strcmp(varname,"ratio")==0) {
    ul=strtoul(data,&ptr,0);
    if (!*ptr) {
      mod_type = _GROUP_RATIO; group->ratio = ul;
    }
  }

  /* commit to backend */
  ret = backend_mod_group(config->backends->filename, group->gid, group, mod_type);

  return ret;
}

int vars_group_new(const char *groupname, wzd_config_t * config)
{
  int err;
  wzd_group_t * newgroup;

  newgroup = group_create(groupname,NULL,config,&err);
  if (newgroup == NULL) return err;

  /* add it to backend */
  err = backend_mod_group(config->backends->filename,0,newgroup,_GROUP_CREATE);

  if (err) { /* problem adding group */
    group_free(newgroup);
  }

  return (err) ? 1 : 0;
}


static unsigned int _str_hash(const char *key)
{
  const char *p = key;
  unsigned int h = *p;

  if (h)
    for (p += 1; *p != '\0'; p++)
      h = (h << 5) - h + *p;
  return h;
}



void vars_shm_init(void)
{
  memset(_shm_vars, 0, sizeof(_shm_vars));
}

void vars_shm_free(void)
{
  unsigned int i;
  struct wzd_shm_vars_t * var, * next_var;

  WZD_MUTEX_LOCK(SET_MUTEX_SHVARS);
  for (i=0; i<32; i++)
  {
    var = _shm_vars[i];
    _shm_vars[i] = 0;

    while (var) {
      if (var->key) {
        wzd_free(var->key);
        wzd_free(var->data);
      }

      next_var = var->next_var;
      wzd_free(var);
      var = next_var;
    }
  }
  WZD_MUTEX_UNLOCK(SET_MUTEX_SHVARS);
}

/* finds shm entry corresponding to 'varname'
 * @returns a pointer to the struct or NULL
 */
struct wzd_shm_vars_t * vars_shm_find(const char *varname, wzd_config_t * config)
{
  unsigned int hash;
  unsigned short index;
  struct wzd_shm_vars_t * var;

  hash = _str_hash(varname);
  index = (hash >> 7) & 31; /* take 5 bits start from the seventh, to give an index in 0 -> 31 */

  var = _shm_vars[index];
  while (var)
  {
    if (strcmp(var->key, varname)==0)
      return var;
  }

  return NULL;
}

/* fills data with varname content, max size: datalength
 * @returns 0 if ok, 1 if an error occured
 */
int vars_shm_get(const char *varname, char *data, size_t datalength, wzd_config_t * config)
{
  struct wzd_shm_vars_t * var;
  int ret = 1;

  WZD_MUTEX_LOCK(SET_MUTEX_SHVARS);
  var = vars_shm_find(varname, config);

  if (var) {
    memcpy(data, var->data, MIN(datalength,var->datalength));
    ret = 0;
  }

  WZD_MUTEX_UNLOCK(SET_MUTEX_SHVARS);
  return ret;
}

/* change varname with data contents size of data is datalength
 * Create varname if needed.
 * @returns 0 if ok, 1 if an error occured
 */
int vars_shm_set(const char *varname, const char *data, size_t datalength, wzd_config_t * config)
{
  struct wzd_shm_vars_t * var;

  WZD_MUTEX_LOCK(SET_MUTEX_SHVARS);

  var = vars_shm_find(varname, config);

  if (!var) { /* new variable, must create it */
    unsigned int hash;
    unsigned short index;

    hash = _str_hash(varname);
    index = (hash >> 7) & 31; /* take 5 bits start from the seventh, to give an index in 0 -> 31 */

    var = wzd_malloc(sizeof(struct wzd_shm_vars_t));
    var->key = wzd_strdup(varname);
    var->data = wzd_malloc(datalength);
    memcpy(var->data, data, datalength);
    var->datalength = datalength;

    /* insertion */
    var->next_var = _shm_vars[index];
    _shm_vars[index] = var;
  } else {
    /* modification */
    if (datalength < var->datalength)
      memcpy(var->data, data, datalength);
    else { /* need to realloc */
      var->data = wzd_realloc(var->data, datalength);
      memcpy(var->data, data, datalength);
      var->datalength = datalength;
    }
  }
  WZD_MUTEX_UNLOCK(SET_MUTEX_SHVARS);

  return 0;
}
