/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <memory>
#include <string>
#include <sstream>

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
                                 PurpleConversation *conv, int flags, void *data,
                                 bool received_from_chat_cb = false)
{
    ImMessage::ImMessageType msg_type = ImMessage::kMsgTypeChat;
    purple_debug_info(PLUGIN_ID, "Got a chat msg (see below):\n");
    std::ostringstream dbg_msg;
    dbg_msg << __FUNCTION__ << "(account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "  %s\n", dbg_msg.str().c_str());
    if (received_from_chat_cb) {
        purple_debug_info(PLUGIN_ID, "  ... received from chat_cb\n");
    }
    if (flags & PURPLE_MESSAGE_NICK) {
        purple_debug_info(PLUGIN_ID, "This is a NICK msg\n");
        msg_type = ImMessage::kMsgTypeChatAcc;
    }
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), msg_type));
    g_msg_history.add_im_message(new_msg);
}


#if 0
static void sent_im_msg_cb(PurpleAccount *account, const char *recipient,
                           const char *buffer, void *data)
{
    std::ostringstream dbg_msg;
    dbg_msg << "sent-im-msg: (account, recipient, buffer, data)" << account << ", "
            << recipient << ", " << buffer << ", " << data;
    purple_debug_info(PLUGIN_ID, "Sent an IM msg: %s\n", dbg_msg.str().c_str());
}
#endif


static void wrote_im_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                            PurpleConversation *conv, int flags, void *data)
{
    std::ostringstream dbg_msg;
    dbg_msg << "wrote-im-msg: (account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "New IM msg in conversation: %s\n",
                      dbg_msg.str().c_str());
    received_im_msg_cb(account, sender, buffer, conv, flags, data);
}


#if 0
void (*wrote_chat_msg)(PurpleAccount *account, const char *who,
                       char *message, PurpleConversation *conv,
                       PurpleMessageFlags flags);
#endif


static void wrote_chat_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                              PurpleConversation *conv, int flags, void *data)
{
    std::ostringstream dbg_msg;
    dbg_msg << "wrote-chat-msg: (account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "New chat msg in conversation: %s\n",
                      dbg_msg.str().c_str());
    received_chat_msg_cb(account, sender, buffer, conv, flags, data);
//    received_chat_msg_cb(account, sender, buffer, conv, flags, data);
}


void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix)
{
    // setup purple callbacks
    //purple_signal_connect(purple_conversations_get_handle(), "received-im-msg", plugin,
    //                      PURPLE_CALLBACK(received_im_msg_cb), NULL);
    purple_signal_connect(purple_conversations_get_handle(), "received-chat-msg", plugin,
                          PURPLE_CALLBACK(received_chat_msg_cb), NULL);
    //purple_signal_connect(purple_conversations_get_handle(), "sent-im-msg", plugin,
    //                      PURPLE_CALLBACK(sent_im_msg_cb), NULL);
    purple_signal_connect(purple_conversations_get_handle(), "wrote-im-msg", plugin,
                          PURPLE_CALLBACK(wrote_im_msg_cb), NULL);
    purple_signal_connect(purple_conversations_get_handle(), "wrote-chat-msg", plugin,
                          PURPLE_CALLBACK(wrote_chat_msg_cb), NULL);

    // setup internal data
    if (url_prefix) {
        g_url_prefix = url_prefix;
    }
}


gboolean timeout_cb(gpointer user_data)
{
    purple_debug_info(PLUGIN_ID, "Timeout callback");
    int type = purple_conversation_get_type(g_send_msg_data.conv);
    switch (type) {
    case PURPLE_CONV_TYPE_IM:
        purple_conv_im_send(PURPLE_CONV_IM(g_send_msg_data.conv),
                            g_send_msg_data.msg.c_str());
        break;

    case PURPLE_CONV_TYPE_CHAT:
        purple_conv_chat_send(PURPLE_CONV_CHAT(g_send_msg_data.conv),
                              g_send_msg_data.msg.c_str());
        break;
    default:
        // :fixme: do something
        break;
    }
    g_send_msg_data.conv = NULL;
    g_send_msg_data.msg.clear();
    return FALSE;
}


struct SendMsgData g_send_msg_data;

void purple_info(const std::string &msg)
{
    purple_debug_info(PLUGIN_ID, "%s\n", msg.c_str());
}
