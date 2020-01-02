/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#define PURPLE_PLUGINS

#include <string.h>
#include <glib.h>
#include <pthread.h>

#include <microhttpd.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"

#include "purple-rest.h"

void perform_rest_request(const char *url, HttpMethod method,
                          const char *upload_data, size_t upload_data_size,
                          char **buf, int *buf_len, char **content_type,
                          int *http_code);

void init_rest_api();

void init_purple_rest_module();

static gboolean do_work(gpointer);

// data used for communication between threads
static pthread_mutex_t rest_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rest_cond = PTHREAD_COND_INITIALIZER;
static int operation_in_progress = 0;
static char *p_url = NULL;
static char *p_stored_upload_data = NULL;
static size_t stored_size = 0;
static char *p_content = NULL, *p_content_type = NULL;
static int content_size = 0, http_code = 200;
static HttpMethod http_method = kHttpMethodUndefined;

// We should not use purple functions from another thread, so using purple_debug_info in
// the http thread is not a good idea.
// Use this only in case of emergency.
//#define DEBUG_POST_REQUEST


void reset_async_data()
{
    free(p_url);
    p_url = NULL;
    content_size = 0;
    http_code = 200;
    http_method = kHttpMethodUndefined;
    free(p_stored_upload_data);
    p_stored_upload_data = NULL;
    stored_size = 0;
    free(p_content);
    p_content = NULL;
    free(p_content_type);
    p_content_type = NULL;
}


/**
 * This will run on a separate thread.
 * We should not call libpurple functions from this thread.
 */
int answer_to_http_connection(
  void *cls, struct MHD_Connection *connection,
  const char *url, const char *method, const char *version,
  const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    static int post_request_in_progress;

    if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
        http_method = kHttpMethodPost;
#ifdef DEBUG_POST_REQUEST
        purple_debug_info(PLUGIN_ID, "POST: upload_data=%p, upload_data_size=%ld\n",
                          upload_data, *upload_data_size);
#endif
        // :fixme: - atm we don't handle multiple pieces of data
        if (post_request_in_progress) {
            if (*upload_data_size != 0) {
                if (upload_data) {
#ifdef DEBUG_POST_REQUEST
                    purple_debug_info(PLUGIN_ID, "POST: storing data\n");
#endif
                    p_stored_upload_data = malloc(*upload_data_size + 1);
                    memcpy(p_stored_upload_data, upload_data, *upload_data_size);
                    p_stored_upload_data[*upload_data_size] = 0;
                    stored_size = *upload_data_size + 1;
                    *upload_data_size = 0;
                    return MHD_YES;
                }
            }
        } else {
#ifdef DEBUG_POST_REQUEST
            purple_debug_info(PLUGIN_ID, "POST: starting POST\n");
#endif
            post_request_in_progress = 1;
            return MHD_YES;
        }
    } else if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
        http_method = kHttpMethodGet;
    } else if (0 == strcmp(method, MHD_HTTP_METHOD_DELETE)) {
        http_method = kHttpMethodDelete;
    } else if (0 == strcmp(method, MHD_HTTP_METHOD_PUT)) {
        http_method = kHttpMethodPut;
    }


    p_url = strdup(url);
    operation_in_progress = 1;
    // :fixme: is there another way to send the message, without using a timer?
    purple_timeout_add(10, do_work, NULL);
    pthread_mutex_lock(&rest_mutex);
    while(operation_in_progress) {
        pthread_cond_wait(&rest_cond, &rest_mutex);
    }
    pthread_mutex_unlock(&rest_mutex);

    struct MHD_Response *response = NULL;
    response = MHD_create_response_from_buffer(content_size, p_content,
                                               MHD_RESPMEM_MUST_COPY);
    if (p_content_type) {
        MHD_add_response_header(response, "Content-Type", p_content_type);
    }
    // use only in case of emergency
#ifdef DEBUG_POST_REQUEST
    purple_debug_info(PLUGIN_ID, "Sending back HTTP response: %d\n", http_code);
    if (response == NULL) {
        purple_debug_warning(PLUGIN_ID, "microhttpd response is NULL\n");
    }
#endif
    MHD_queue_response(connection, http_code, response);
    MHD_destroy_response(response);

    if (post_request_in_progress) {
        post_request_in_progress = 0;
    }
    reset_async_data();
    return MHD_YES;
}


/**
 * This will run in libpurple main thread.
 */
static gboolean do_work(gpointer ignored)
{
    perform_rest_request(p_url, http_method, p_stored_upload_data, stored_size,
                         &p_content, &content_size, &p_content_type, &http_code);
    pthread_mutex_lock(&rest_mutex);
    operation_in_progress = 0;
    pthread_cond_signal(&rest_cond);
    pthread_mutex_unlock(&rest_mutex);
    return FALSE; // to stop the timer from firing again
}


static gboolean plugin_load(PurplePlugin *plugin)
{
    purple_debug_info(PLUGIN_ID, "Purple REST plugin is up & running\n");

    // read preferences
    int http_port = purple_prefs_get_int("/plugins/core/purple-rest/server-port");
    const char *p = purple_prefs_get_string("/plugins/core/purple-rest/url-prefix");
    purple_debug_info(PLUGIN_ID, "server-port: %d\n", http_port);
    purple_debug_info(PLUGIN_ID, "url-prefix: %s\n", p ? p : "NULL");
    if (http_port == 0) {
        http_port = 8888;
    }
    purple_debug_info(PLUGIN_ID, "server-port (real): %d\n", http_port);

    init_rest_api();

    // setup HTTP server
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, http_port, NULL, NULL,
                               &answer_to_http_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        return FALSE;
    }

    init_purple_rest_module(plugin, p);

    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL, /* this must be NULL for core plugins */
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    PLUGIN_ID,
    "Purple REST",
    "0.1",

    "libpurple REST Plugin",
    "libpurple REST Plugin",
    "Tudor Pristavu <clapautius gmail com>",
    "https://github.com/clapautius/purple-rest",

    plugin_load,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


static void init_plugin(PurplePlugin *plugin)
{

}

PURPLE_INIT_PLUGIN(hello_world, init_plugin, info)
