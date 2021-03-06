#include <stdlib.h> /* malloc */
#include <string.h> /* memset */

#include <libwzd-core/wzd_structs.h>
#include <libwzd-core/wzd_group.h>
#include <libwzd-core/wzd_libmain.h>
#include <libwzd-core/wzd_log.h>
#include <libwzd-core/wzd_mod.h>
#include <libwzd-core/wzd_user.h>
#include <libwzd-core/wzd_utf8.h>

#include <libwzd-core/wzd_cache.h>

#include <libwzd-core/wzd_debug.h>

#include "test_common.h"
#include "fake_backend.h"

static const char * b_name = "fake_backend";
static const unsigned int b_version = 100;

static int fb_init(const char * param);
static wzd_user_t * fb_get_user(uid_t uid);
static wzd_group_t * fb_get_group(gid_t gid);


int fake_backend_init(wzd_backend_t * b)
{
  if (b == NULL) return -1;

  b->name = wzd_strdup(b_name);
  b->version = b_version;

  b->backend_init = fb_init;

  b->backend_get_user = fb_get_user;
  b->backend_get_group = fb_get_group;

  return 0;
}

static int fb_init(const char * param)
{
  out_log(LEVEL_INFO,"DEBUG fake backend init called\n");
  return 0;
}

static wzd_user_t * fb_get_user(uid_t uid)
{
  if (f_user->uid == uid) return f_user;

  return NULL;
}

static wzd_group_t * fb_get_group(gid_t gid)
{
  if (f_group->gid == gid) return f_group;

  return NULL;
}

