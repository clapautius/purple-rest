#include <memory>
#include <string>

#include "purple-interaction.hpp"

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"

#include "purple-rest.h"
#include "immessage.hpp"
#include "history.hpp"

using std::shared_ptr;
using purple::ImMessage;

// :fixme: - don't use globals
purple::History g_msg_history;


static void received_im_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                               PurpleConversation *conv, int flags, void *data)
{
    purple_debug_info(PLUGIN_ID, "Got an IM msg: %s", buffer);
    shared_ptr<ImMessage> new_msg(new ImMessage(account, buffer));
    g_msg_history.add_im_message(new_msg);
}


static void received_chat_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                                 PurpleConversation *conv, int flags, void *data)
{
    purple_debug_info(PLUGIN_ID, "Got a chat msg: %s", buffer);
}


void init_purple_rest_module(PurplePlugin *plugin)
{
    // setup purple callbacks
    purple_signal_connect(purple_conversations_get_handle(), "received-im-msg", plugin,
                          PURPLE_CALLBACK(received_im_msg_cb), NULL);
    purple_signal_connect(purple_conversations_get_handle(), "received-chat-msg", plugin,
                          PURPLE_CALLBACK(received_chat_msg_cb), NULL);

}


void purple_info(const std::string &msg)
{
    purple_debug_info(PLUGIN_ID, "%s", msg.c_str());
}
