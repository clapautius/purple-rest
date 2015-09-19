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


int answer_to_http_connection (void *cls, struct MHD_Connection *connection,
                               const char *url,
                               const char *method, const char *version,
                               const char *upload_data,
                               size_t *upload_data_size, void **con_cls)
{
    char page[1024];
    perform_rest_request(page, sizeof(page));
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_buffer(strlen (page),
                                               (void*) page, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "text/html");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}


static gboolean plugin_load(PurplePlugin *plugin)
{
    purple_debug_info(PLUGIN_ID, "Purple REST plugin is up & running");

    // setup HTTP server
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL,
                               &answer_to_http_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        return FALSE;
    }

    init_purple_rest_module(plugin);

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
