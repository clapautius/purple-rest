/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#define PURPLE_PLUGINS

#include <string.h>
#include <glib.h>

#include <microhttpd.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"

#include "purple-rest.h"

void init_purple_rest_module();


int answer_to_http_connection(
  void *cls, struct MHD_Connection *connection,
  const char *url, const char *method, const char *version,
  const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    static int post_request_in_progress;
    static char *stored_upload_data ;
    static size_t stored_size;
    char *content = NULL, *content_type = NULL;
    int content_size = 0, http_code = 200;
    HttpMethod http_method = kHttpMethodGet;

    if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
        http_method = kHttpMethodPost;
        purple_debug_info(PLUGIN_ID, "POST: upload_data=%p, upload_data_size=%ld\n",
                          upload_data, *upload_data_size);
        // :fixme: - atm we don't handle multiple pieces of data
        if (post_request_in_progress) {
            if (*upload_data_size == 0) {
                purple_debug_info(PLUGIN_ID, "POST: calling the function\n");
            } else {
                if (upload_data) {
                    purple_debug_info(PLUGIN_ID, "POST: storing data\n");
                    stored_upload_data = malloc(*upload_data_size + 1);
                    memcpy(stored_upload_data, upload_data, *upload_data_size);
                    stored_upload_data[*upload_data_size] = 0;
                    stored_size = *upload_data_size + 1;
                    *upload_data_size = 0;
                    return MHD_YES;
                }
            }
        } else {
            purple_debug_info(PLUGIN_ID, "POST: starting POST\n");
            post_request_in_progress = 1;
            return MHD_YES;
        }
    } else if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
        http_method = kHttpMethodGet;
    } else if (0 == strcmp(method, MHD_HTTP_METHOD_DELETE)) {
        http_method = kHttpMethodDelete;
    }

    perform_rest_request(url, http_method, stored_upload_data, stored_size,
                         &content, &content_size, &content_type, &http_code);
    if (post_request_in_progress) {
        post_request_in_progress = 0;
        free(stored_upload_data);
        stored_upload_data = NULL;
        stored_size = 0;
    }
    struct MHD_Response *response = NULL;
    response = MHD_create_response_from_buffer(content_size, content,
                                               MHD_RESPMEM_MUST_COPY);
    if (content_type) {
        MHD_add_response_header(response, "Content-Type", content_type);
    }
    purple_debug_info(PLUGIN_ID, "Sending back HTTP response: %d\n", http_code);
    if (response == NULL) {
        purple_debug_warning(PLUGIN_ID, "microhttpd response is NULL\n");
    }
    MHD_queue_response(connection, http_code, response);
    MHD_destroy_response(response);
    free(content);
    free(content_type);
    return MHD_YES;
}


static gboolean plugin_load(PurplePlugin *plugin)
{
    purple_debug_info(PLUGIN_ID, "Purple REST plugin is up & running\n");

    // read preferences
    int http_port = purple_prefs_get_int("/plugins/purple-rest/server-port");
    const char *p = purple_prefs_get_string("/plugins/purple-rest/url-prefix");
    purple_debug_info(PLUGIN_ID, "server-port: %d\n", http_port);
    purple_debug_info(PLUGIN_ID, "url-prefix: %s\n", p ? p : "NULL");
    if (http_port == 0) {
        http_port = 8888;
    }
    purple_debug_info(PLUGIN_ID, "server-port (real): %d\n", http_port);

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
