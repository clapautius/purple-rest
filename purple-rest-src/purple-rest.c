#define PURPLE_PLUGINS

#include <glib.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"

#define PLUGIN_ID "core-purple_rest"


static gboolean
plugin_load(PurplePlugin *plugin) {
    purple_debug_info(PLUGIN_ID, "Purple REST plugin is up & running");
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
