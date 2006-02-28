/* vi:ai:et:ts=8 sw=2
 */
/*
 * wzdftpd - a modular and cool ftp server
 * Copyright (C) 2002-2006  Pierre Chifflier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_AVAHI

#include "libwzd_avahi.h"

/*
 * This function tries to register the FTP DNS
 * SRV service type.
 */
static void register_stuff(struct context *ctx) {
  char r[128];
  int ret;

  assert(ctx->client);

  if (!ctx->group) {

    if (!(ctx->group = avahi_entry_group_new(ctx->client,
                                             publish_reply,
                                             ctx))) {
      out_log(LEVEL_CRITICAL,
              "Failed to create entry group: %s\n",
              avahi_strerror(avahi_client_errno(ctx->client)));
      goto fail;
    }

  }

  out_log(LEVEL_INFO, "Adding service '%s'\n", ctx->name);

  if (avahi_entry_group_is_empty(ctx->group)) {
    /* Register our service */

    if (avahi_entry_group_add_service(ctx->group,
                                      AVAHI_IF_UNSPEC,
                                      AVAHI_PROTO_UNSPEC,
                                      0,
                                      ctx->name,
                                      FTP_DNS_SERVICE_TYPE,
                                      NULL,
                                      NULL,
                                      ctx->port,
                                      NULL) < 0) {
      out_log(LEVEL_CRITICAL,
              "Failed to add service: %s\n",
              avahi_strerror(avahi_client_errno(ctx->client)));
      goto fail;
    }

    if (avahi_entry_group_commit(ctx->group) < 0) {
      out_log(LEVEL_CRITICAL,
              "Failed to commit entry group: %s\n",
              avahi_strerror(avahi_client_errno(ctx->client)));
      goto fail;
    }
  }

  return;

  fail:
    avahi_client_free (ctx->client);
#ifndef HAVE_AVAHI_THREADED_POLL
    avahi_simple_poll_quit(ctx->simple_poll);
#else
    avahi_threaded_poll_quit(ctx->threaded_poll);
#endif
}

/* Called when publishing of service data completes */
static void publish_reply(AvahiEntryGroup *g,
                          AvahiEntryGroupState state,
                          AVAHI_GCC_UNUSED void *userdata)
{
  struct context *ctx = userdata;

  assert(g == ctx->group);

  switch (state) {

  case AVAHI_ENTRY_GROUP_ESTABLISHED :
    /* The entry group has been established successfully */
    break;

  case AVAHI_ENTRY_GROUP_COLLISION: {
    char *n;

    /* Pick a new name for our service */

    n = avahi_alternative_service_name(ctx->name);
    assert(n);

    avahi_free(ctx->name);
    ctx->name = n;

    register_stuff(ctx);
    break;
  }

  case AVAHI_ENTRY_GROUP_FAILURE: {
    out_log(LEVEL_CRITICAL,
            "Failed to register service: %s\n",
            avahi_strerror(avahi_client_errno(ctx->client)));
    avahi_client_free (avahi_entry_group_get_client(g));
#ifndef HAVE_AVAHI_THREADED_POLL
    avahi_simple_poll_quit(ctx->simple_poll);
#else
    avahi_threaded_poll_quit(ctx->threaded_poll);
#endif
    break;
  }

  case AVAHI_ENTRY_GROUP_UNCOMMITED:
  case AVAHI_ENTRY_GROUP_REGISTERING:
    ;
  }
}

static void client_callback(AvahiClient *client,
                            AvahiClientState state,
                            void *userdata)
{
  struct context *ctx = userdata;

  ctx->client = client;

  switch (state) {

  case AVAHI_CLIENT_S_RUNNING:

    /* The server has startup successfully and registered its host
     * name on the network, so it's time to create our services */
    if (!ctx->group)
      register_stuff(ctx);
    break;

  case AVAHI_CLIENT_S_COLLISION:

    if (ctx->group)
      avahi_entry_group_reset(ctx->group);
    break;

  case AVAHI_CLIENT_FAILURE: {

    if (avahi_client_errno(client) == AVAHI_ERR_DISCONNECTED) {
      int error;

      avahi_client_free(ctx->client);
      ctx->client = NULL;
      ctx->group = NULL;

      /* Reconnect to the server */

#ifndef HAVE_AVAHI_THREADED_POLL
      if (!(ctx->client = avahi_client_new(avahi_simple_poll_get(ctx->simple_poll),
#else
      if (!(ctx->client = avahi_client_new(avahi_threaded_poll_get(ctx->threaded_poll),
#endif
                                           AVAHI_CLIENT_NO_FAIL,
                                           client_callback,
                                           ctx,
                                           &error))) {

        out_log(LEVEL_CRITICAL, "Failed to contact server: %s\n", avahi_strerror(error));

        avahi_client_free (ctx->client);
#ifndef HAVE_AVAHI_THREADED_POLL
        avahi_simple_poll_quit(ctx->simple_poll);
#else
        avahi_threaded_poll_quit(ctx->threaded_poll);
#endif
      }

      } else {
        out_log(LEVEL_CRITICAL, "Client failure: %s\n", avahi_strerror(avahi_client_errno(client)));

        avahi_client_free (ctx->client);
#ifndef HAVE_AVAHI_THREADED_POLL
        avahi_simple_poll_quit(ctx->simple_poll);
#else
        avahi_threaded_poll_quit(ctx->threaded_poll);
#endif
      }

    break;
  }

  case AVAHI_CLIENT_S_REGISTERING:
  case AVAHI_CLIENT_CONNECTING:
    ;
  }
}

/*
 * Tries to setup the Zeroconf thread and any
 * neccessary config setting.
 */
void* av_zeroconf_setup(unsigned long port, const char *name) {
  struct context *ctx = NULL;

  /* default service name, if there's none in
   * the config file.
   */
  char service[256] = "WZDFTP Server on ";
  int error, ret;

  /* initialize the struct that holds our
   * config settings.
   */
  ctx = malloc(sizeof(struct context));
  assert(ctx);
  ctx->client = NULL;
  ctx->group = NULL;
#ifndef HAVE_AVAHI_THREADED_POLL
  ctx->simple_poll = NULL;
#else
  ctx->threaded_poll = NULL;
#endif
  ctx->thread_running = 0;
  ctx->port = port;
  pthread_mutex_init(&ctx->mutex, NULL);

  /* Prepare service name */
  if (!name) {
    out_log(LEVEL_INFO, "Assigning default service name.\n");
    gethostname(service+17, sizeof(service)-18);
    service[sizeof(service)-1] = 0;

    ctx->name = strdup(service);
  }
  else {
    ctx->name = strdup(name);
  }

  assert(ctx->name);

/* first of all we need to initialize our threading env */
#ifdef HAVE_AVAHI_THREADED_POLL
  if (!(ctx->threaded_poll = avahi_threaded_poll_new())) {
     return;
  }
#else
  avahi_simple_poll_set_func(ctx->simple_poll, poll_func, &ctx->mutex);

  if (!(ctx->simple_poll = avahi_simple_poll_new())) {
      out_log(LEVEL_CRITICAL, "Failed to create event loop object.\n");
      goto fail;
  }
#endif

/* now we need to acquire a client */
#ifdef HAVE_AVAHI_THREADED_POLL
  if (!(ctx->client = avahi_client_new(avahi_threaded_poll_get(ctx->threaded_poll),
                                       AVAHI_CLIENT_NO_FAIL,
                                       client_callback,
                                       ctx,
                                       &error))) {
    out_log(LEVEL_CRITICAL,
            "Failed to create client object: %s\n",
            avahi_strerror(avahi_client_errno(ctx->client)));
    goto fail;
  }
#else
  if (!(ctx->client = avahi_client_new(avahi_simple_poll_get(ctx->simple_poll),
                                       AVAHI_CLIENT_NO_FAIL,
                                       client_callback,
                                       ctx,
                                       &error))) {
    out_log(LEVEL_CRITICAL,
            "Failed to create client object: %s\n",
            avahi_strerror(avahi_client_errno(ctx->client)));
    goto fail;
  }
#endif

  return ctx;

fail:

  if (ctx)
    av_zeroconf_unregister(ctx);

  return NULL;
}

/*
 * This function finally runs the loop impl.
 */
int av_zeroconf_run(void *u) {
  struct context *ctx = u;
  int ret;

#ifdef HAVE_AVAHI_THREADED_POLL
  /* Finally, start the event loop thread */
  if (avahi_threaded_poll_start(ctx->threaded_poll) < 0) {
    out_log(LEVEL_CRITICAL,
            "Failed to create thread: %s\n",
            avahi_strerror(avahi_client_errno(ctx->client)));
    goto fail;
  } else {
    out_log(LEVEL_INFO, "Successfully started avahi loop.\n");
  }
#else
  /* Create the mDNS event handler */
  if ((ret = pthread_create(&ctx->thread_id, NULL, thread, ctx)) < 0) {
    out_log(LEVEL_CRITICAL, "Failed to create thread: %s\n", strerror(ret));
    goto fail;
  } else {
    out_log(LEVEL_INFO, "Successfully started avahi loop.\n");
  }
#endif

  ctx->thread_running = 1;

  return 0;

fail:

  if (ctx)
    av_zeroconf_unregister(ctx);

  return -1;
}

/*
 * Used to lock access to the loop.
 * Currently unused.
 */
void av_zeroconf_lock(void *u) {
  struct context *ctx = u;

  avahi_threaded_poll_lock(ctx->threaded_poll);
}

/*
 * Used to unlock access to the loop.
 * Currently unused.
 */
void av_zeroconf_unlock(void *u) {
  struct context *ctx = u;

  avahi_threaded_poll_unlock(ctx->threaded_poll);
}

/*
 * Tries to shutdown this loop impl.
 * Call this function from outside this thread.
 */
void av_zeroconf_shutdown(void *u) {
  struct context *ctx = u;

  /* Call this when the app shuts down */
#ifdef HAVE_AVAHI_THREADED_POLL
  avahi_threaded_poll_stop(ctx->threaded_poll);
  avahi_free(ctx->name);
  avahi_client_free(ctx->client);
  avahi_threaded_poll_free(ctx->threaded_poll);
#else
  av_zeroconf_unregister(ctx);
#endif
}

/*
 * Tries to shutdown this loop impl.
 * Call this function from inside this thread.
 */
int av_zeroconf_unregister(void *u) {
  struct context *ctx = u;

  if (ctx->thread_running) {
#ifndef HAVE_AVAHI_THREADED_POLL
    pthread_mutex_lock(&ctx->mutex);
    avahi_simple_poll_quit(ctx->simple_poll);
    pthread_mutex_unlock(&ctx->mutex);

    pthread_join(ctx->thread_id, NULL);
#else
    /* First, block the event loop */
    avahi_threaded_poll_lock(ctx->threaded_poll);

    /* Than, do your stuff */
    avahi_threaded_poll_quit(ctx->threaded_poll);

    /* Finally, unblock the event loop */
    avahi_threaded_poll_unlock(ctx->threaded_poll);
#endif
    ctx->thread_running = 0;
  }

  avahi_free(ctx->name);

  if (ctx->client)
    avahi_client_free(ctx->client);

#ifndef HAVE_AVAHI_THREADED_POLL
  if (ctx->simple_poll)
    avahi_simple_poll_free(ctx->simple_poll);

  pthread_mutex_destroy(&ctx->mutex);
#else
  if (ctx->threaded_poll)
    avahi_threaded_poll_free(ctx->threaded_poll);
#endif

  free(ctx);

  return 0;
}

#endif /* USE_AVAHI */
