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
#include "idle.h"

#include "purple-rest.h"
#include "immessage.hpp"
#include "imconversation.hpp"
#include "history.hpp"

using std::shared_ptr;
using p_rest::ImMessage;
using p_rest::History;
using p_rest::g_conv_list;

p_rest::History g_msg_history;
std::string g_url_prefix;


static void received_im_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                               PurpleConversation *conv, int flags, void *data)
{
    purple_debug_info(PLUGIN_ID, "Got an IM msg: %s\n", buffer);
    ImMessage::ImMessageType msg_type = ImMessage::kMsgTypeIm;
    if (flags & PURPLE_MESSAGE_SYSTEM) {
        purple_debug_info(PLUGIN_ID, "This is a SYSTEM msg\n");
        msg_type = ImMessage::kMsgTypeSystem;
    }
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), msg_type));
    g_msg_history.add_im_message(new_msg);
}


#if 0
static void received_chat_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                                 PurpleConversation *conv, int flags, void *data)
{
    ImMessage::ImMessageType msg_type = ImMessage::kMsgTypeChat;
    purple_debug_info(PLUGIN_ID, "Got a chat msg (see below):\n");
    std::ostringstream dbg_msg;
    dbg_msg << __FUNCTION__ << "(account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "  %s\n", dbg_msg.str().c_str());
    if (flags & PURPLE_MESSAGE_NICK) {
        purple_debug_info(PLUGIN_ID, "This is a NICK msg\n");
        msg_type = ImMessage::kMsgTypeChatAcc;
    }
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), msg_type));
    g_msg_history.add_im_message(new_msg);
}


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
    purple_debug_info(PLUGIN_ID, "Got a chat msg (see below):\n");
    dbg_msg << "wrote-chat-msg: (account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "New chat msg in conversation: %s\n",
                      dbg_msg.str().c_str());

    ImMessage::ImMessageType msg_type = ImMessage::kMsgTypeChat;
    if (flags & PURPLE_MESSAGE_NICK) {
        purple_debug_info(PLUGIN_ID, "This is a NICK msg\n");
        msg_type = ImMessage::kMsgTypeChatAcc;
    }
    if (flags & PURPLE_MESSAGE_SYSTEM) {
        purple_debug_info(PLUGIN_ID, "This is a SYSTEM msg\n");
        msg_type = ImMessage::kMsgTypeSystem;
    }
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), msg_type));
    g_msg_history.add_im_message(new_msg);
}


void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix)
{
    // setup purple callbacks
    //purple_signal_connect(purple_conversations_get_handle(), "received-im-msg", plugin,
    //                      PURPLE_CALLBACK(received_im_msg_cb), NULL);
    //purple_signal_connect(purple_conversations_get_handle(), "received-chat-msg",
    //                      plugin, PURPLE_CALLBACK(received_chat_msg_cb), NULL);
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


namespace libpurple
{

void purple_info(const std::string &msg)
{
    purple_debug_info(PLUGIN_ID, "%s\n", msg.c_str());
}


std::string buddy_get_group_name(PurpleBlistNode *p)
{
    if (PURPLE_BLIST_NODE_IS_GROUP(p)) {
        return std::string(PURPLE_GROUP(p)->name ? PURPLE_GROUP(p)->name : "");
    } else {
        if (p->parent) {
            return buddy_get_group_name(p->parent);
        } else {
            return "";
        }
    }
}


void collect_buddies(PurpleBlistNode *p, std::vector<p_rest::Buddy> &list,
                     bool online_only)
{
    if (PURPLE_BLIST_NODE_IS_BUDDY(p)) {
        if (!online_only || (online_only && PURPLE_BUDDY_IS_ONLINE(PURPLE_BUDDY(p)))) {
            p_rest::Buddy new_buddy(PURPLE_BUDDY(p)->name);
            new_buddy.set_group(buddy_get_group_name(p).c_str());
            if (PURPLE_BUDDY_IS_ONLINE(PURPLE_BUDDY(p))) {
                new_buddy.set_online_status(true);
            }
            list.push_back(new_buddy);
        }
    }
    if (p->child) {
        collect_buddies(p->child, list, online_only);
    }
    if (p->next) {
        collect_buddies(p->next, list, online_only);
    }
}


void visit_buddies(std::function<bool(PurpleBuddy*)> func, PurpleBlistNode *p = nullptr)
{
    if (p == nullptr) {
        p = purple_get_blist()->root;
    }
    if (PURPLE_BLIST_NODE_IS_BUDDY(p)) {
        if (func(PURPLE_BUDDY(p))) {
            // found something, stop searching
            return;
        }
    }
    if (p->child) {
        visit_buddies(func, p->child);
    }
    if (p->next) {
        visit_buddies(func, p->next);
    }
}


std::string get_account_status()
{
    std::string result;
    // :fixme:
    // atm we get the status only for the first account (assuming the status is
    // identical forall active accounts - which may not be true)
    GList *p_accounts = purple_accounts_get_all_active();
    if (p_accounts) {
        PurpleStatus *p_status = purple_account_get_active_status(
          reinterpret_cast<PurpleAccount*>(g_list_first(p_accounts)->data));
        const char *p_status_text = purple_primitive_get_name_from_type(
          purple_status_type_get_primitive(purple_status_get_type(p_status)));
        if (p_status_text) {
            result = p_status_text;
        } else {
            result = "Unknown";
            }
    } else {
        // no active accounts -> offline
        result = "Offline";
    }
    return result;
}


void reset_idle()
{
    purple_idle_touch();
}


PurpleBuddy* get_buddy_by_name(const std::string &buddy_name)
{
    PurpleBuddy *ret = nullptr;
    visit_buddies([&] (PurpleBuddy *p) -> bool {
            if (buddy_name == p->name) {
                ret = p;
                return true;
            } else {
                return false;
            }});
    return ret;
}

}
