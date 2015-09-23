/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <memory>
#include <string>

#include "purple-interaction.hpp"

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"

#include "purple-rest.h"
#include "immessage.hpp"
#include "imconversation.hpp"
#include "history.hpp"

using std::shared_ptr;
using purple::ImMessage;
using purple::History;
using purple::g_conv_list;

// :fixme: - don't use globals
purple::History g_msg_history;

std::string g_url_prefix;


static void received_im_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                               PurpleConversation *conv, int flags, void *data)
{
    purple_debug_info(PLUGIN_ID, "Got an IM msg: %s\n", buffer);
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), ImMessage::kMsgTypeIm));
    g_msg_history.add_im_message(new_msg);
}


static void received_chat_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                                 PurpleConversation *conv, int flags, void *data)
{
    purple_debug_info(PLUGIN_ID, "Got a chat msg: %s\n", buffer);
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), ImMessage::kMsgTypeChat));
    g_msg_history.add_im_message(new_msg);
}


void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix)
{
    // setup purple callbacks
    purple_signal_connect(purple_conversations_get_handle(), "received-im-msg", plugin,
                          PURPLE_CALLBACK(received_im_msg_cb), NULL);
    purple_signal_connect(purple_conversations_get_handle(), "received-chat-msg", plugin,
                          PURPLE_CALLBACK(received_chat_msg_cb), NULL);

    // setup internal data
    if (url_prefix) {
        g_url_prefix = url_prefix;
    }
}


void purple_info(const std::string &msg)
{
    purple_debug_info(PLUGIN_ID, "%s\n", msg.c_str());
}
