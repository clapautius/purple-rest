/* Purple REST plugin -- Copyright (C) 2019-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <memory>
#include <string>
#include <sstream>
#include <map>
#include <vector>

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
using std::string;
using std::vector;
using p_rest::ImMessage;
using p_rest::History;
using p_rest::g_conv_list;

p_rest::History g_msg_history;
std::string g_url_prefix;


static void wrote_im_msg_cb(PurpleAccount *account, char *sender, char *buffer,
                            PurpleConversation *conv, int flags, void *data)
{
    std::ostringstream dbg_msg;
    dbg_msg << "wrote-im-msg: (account, sender, buffer, conv, flags, data)"
            << account << "," << sender << "," << buffer << ","
            << conv << "," << flags << "," << data;
    purple_debug_info(PLUGIN_ID, "New IM msg in conversation: %s\n",
                      dbg_msg.str().c_str());

    ImMessage::ImMessageType msg_type = ImMessage::kMsgTypeIm;
    if (flags & PURPLE_MESSAGE_SYSTEM) {
        purple_debug_info(PLUGIN_ID, "This is a SYSTEM msg\n");
        msg_type = ImMessage::kMsgTypeSystem;
    }
    shared_ptr<ImMessage> new_msg
      (new ImMessage(account, buffer, History::get_new_id(), sender,
                     g_conv_list.get_or_add_conversation(conv), msg_type, flags));
    g_msg_history.add_im_message(new_msg);
}


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
                     g_conv_list.get_or_add_conversation(conv), msg_type, flags));
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


PurpleAccount* get_account_by_name(const std::string &account_name,
    bool only_active)
{
    GList *p_accounts = nullptr;
    if (only_active) {
        p_accounts = purple_accounts_get_all_active();
    } else {
        p_accounts = purple_accounts_get_all();
    }
    std::string purple_acc_name;
    PurpleAccount *p_acc = nullptr;
    PurpleAccount *p_result = nullptr;
    while (p_accounts) {
        p_acc = reinterpret_cast<PurpleAccount*>(p_accounts->data);
        purple_acc_name = get_purple_account_name(p_acc);
        if (purple_acc_name[purple_acc_name.size() - 1] == '/') {
            purple_acc_name.erase(purple_acc_name.size() - 1);
        }
        if (purple_acc_name == account_name) {
            p_result = p_acc;
            break;
        }
        p_accounts = g_list_next(p_accounts);
    }
    return p_result;
}



std::string get_status_for_account(PurpleAccount *p_account)
{
    string status;
    if (p_account) {
        PurpleStatus *p_status = purple_account_get_active_status(p_account);
        const char *p_status_text = purple_primitive_get_name_from_type(
          purple_status_type_get_primitive(purple_status_get_type(p_status)));
        if (p_status_text) {
            status = p_status_text;
        } else {
            status = "Unknown";
        }
    }
    return status;
}


std::string get_status_for_account(const std::string &account_name,
                                   bool only_active)
{
    string status;
    PurpleAccount *p_account = get_account_by_name(account_name, only_active);
    if (p_account) {
        purple_debug_info(PLUGIN_ID, "Found matching account for %s\n",
                          account_name.c_str());
        status = get_status_for_account(p_account);
    } else {
        purple_debug_info(PLUGIN_ID, "Couldn't find matching account for %s\n",
                          account_name.c_str());
    }
    return status;
}


vector<string> get_statuses_for_account(const PurpleAccount *p_account,
                                        bool debug_on)
{
    std::vector<string> result;
    std::string status;
    if (p_account) {
        PurplePresence *p_presence = purple_account_get_presence(p_account);
        GList *p_statuses = purple_presence_get_statuses(p_presence);
        const char *p_status_text = nullptr;
        while (p_statuses) {
            PurpleStatus *p_stat = reinterpret_cast<PurpleStatus*>(p_statuses->data);
            p_status_text = purple_primitive_get_name_from_type(
              purple_status_type_get_primitive(purple_status_get_type(p_stat)));
            if (p_status_text) {
                status = p_status_text;
            } else {
                status = "Unknown";
            }
            if (debug_on) {
                if (purple_status_is_active(p_stat)) {
                    status += " (active)";
                }
                if (purple_status_is_exclusive(p_stat)) {
                    status += " (exclusive)";
                }
                if (purple_status_is_independent(p_stat)) {
                    status += " (independent)";
                }
            }
            purple_debug_info(PLUGIN_ID, "Status: %s\n", status.c_str());
            result.push_back(status);
            p_statuses = g_list_next(p_statuses);
        }
        purple_debug_info(PLUGIN_ID, "End of statuses\n");
    }
    return result;
}


std::vector<std::string> get_statuses_for_account(const std::string &account_name,
                                                  bool debug_on, bool only_active)
{
    std::vector<string> result;
    PurpleAccount *p_account = get_account_by_name(account_name, only_active);
    if (p_account) {
        purple_debug_info(PLUGIN_ID, "Found matching account for %s\n",
                          account_name.c_str());
        result = get_statuses_for_account(p_account, debug_on);
    } else {
        purple_debug_info(PLUGIN_ID, "Couldn't find matching account for %s\n",
                          account_name.c_str());
    }
    return result;
}


std::map<std::string, std::string> get_status_for_accounts()
{
    std::map<std::string, std::string > result;
    GList *p_accounts = purple_accounts_get_all_active();
    std::string status;
    std::string acc_name;
    while (p_accounts) {
        PurpleAccount *p_acc =
          reinterpret_cast<PurpleAccount*>(p_accounts->data);
        PurpleStatus *p_status = purple_account_get_active_status(p_acc);
        const char *p_status_text = purple_primitive_get_name_from_type(
          purple_status_type_get_primitive(purple_status_get_type(p_status)));
        if (p_status_text) {
            status = p_status_text;
        } else {
            status = "Unknown";
        }
        acc_name = get_purple_account_name(p_acc);
        result[acc_name] = status;
        p_accounts = g_list_next(p_accounts);
    }
    return result;
}


bool set_status_for_all_accounts(const std::string &status)
{
    bool rc = true;
    PurpleStatusPrimitive new_status_primitive = get_primitive_from_status_string(status);
    if (new_status_primitive == PURPLE_STATUS_UNSET) {
        rc = false;
        purple_debug_info(PLUGIN_ID, "Invalid status string: %s\n", status.c_str());
    }

    if (rc) {
        purple_debug_info(PLUGIN_ID, "Status primitive is : %d\n", new_status_primitive);
        GList *p_accounts = purple_accounts_get_all_active();
        PurpleAccount *p_acc = nullptr;
        while (p_accounts) {
            p_acc = reinterpret_cast<PurpleAccount*>(p_accounts->data);
            purple_debug_info(PLUGIN_ID, "Setting status for %s\n",
                              get_purple_account_name(p_acc).c_str());
            if (set_status_for_account(p_acc, new_status_primitive)) {
                purple_debug_info(PLUGIN_ID, "Status changed");
            } else {
                purple_debug_info(PLUGIN_ID, "Could not change status");
                // don't set rc; just continue with the rest of the accounts.
            }
            p_accounts = g_list_next(p_accounts);
        }
    }
    return rc;
}



/**
 * Set status for the specified account.
 *
 * @param[in] account_name: account name (without trailing '/')
 * @param[in] status: one of: 'available', 'away', 'invisible', 'offline'. (WIP)
 *
 * @return true if ok, false on error.
 */
bool set_status_for_account(const std::string &account_name, const std::string &status,
                            bool only_active)
{
    bool rc = true;
    PurpleStatusPrimitive new_status_primitive = get_primitive_from_status_string(status);
    if (new_status_primitive == PURPLE_STATUS_UNSET) {
        rc = false;
        purple_debug_info(PLUGIN_ID, "Invalid status string: %s\n", status.c_str());
    }

    if (rc) {
        purple_debug_info(PLUGIN_ID, "Status primitive is : %d\n", new_status_primitive);
        PurpleAccount *p_acc = get_account_by_name(account_name, only_active);
        if (p_acc) {
            rc = set_status_for_account(p_acc, new_status_primitive);
        } else {
            purple_debug_info(PLUGIN_ID, "No such account: %s\n", account_name.c_str());
            rc = false;
        }
    }
    return rc;
}


/**
 * Set status for the specified account.
 *
 * @param[in] account_name: account name (without trailing '/')
 * @param[in] status: one of: 'available', 'away', 'invisible', 'offline'. (WIP)
 *
 * @return true if ok, false on error.
 */
bool set_status_for_account(const PurpleAccount *p_acc,
                            PurpleStatusPrimitive new_status_primitive)
{
    bool rc = true;
    if (p_acc) {
        // :fixme: atm it's just for debug, ignore result
        get_statuses_for_account(p_acc, true);
        // get the presence of the account
        PurplePresence *p_presence = purple_account_get_presence(p_acc);
        bool found = statuses_iterate_till_true(
          p_acc, [&] (const PurpleStatus *p_stat) {
                     PurpleStatusType *p_type = purple_status_get_type(p_stat);
                     PurpleStatusPrimitive primitive = purple_status_type_get_primitive(p_type);
                     if (primitive == new_status_primitive) {
                         purple_presence_switch_status(
                           p_presence,
                           purple_status_get_id(p_stat));
                         return true;
                     } else {
                         return false;
                     }
                 });
        if (found) {
            purple_debug_info(PLUGIN_ID, "Status changed");
            rc = true;
        } else {
            purple_debug_info(PLUGIN_ID, "Cannot change status, not found");
            rc = false;
        }
    } else {
        purple_debug_info(PLUGIN_ID, "Account is NULL\n");
        rc = false;
    }
    return rc;
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


std::string get_purple_account_name(const PurpleAccount *p_account)
{
    if (p_account->alias) {
        return std::string(p_account->alias);
    } else if (p_account->username) {
        return std::string(p_account->username);
    } else {
        return "Unknown_account";
    }
}


PurpleStatusPrimitive get_primitive_from_status_string(const string &status)
{
    PurpleStatusPrimitive new_status_primitive = PURPLE_STATUS_UNSET;
    if (status == "available") {
        new_status_primitive = PURPLE_STATUS_AVAILABLE;
    } else if (status == "away") {
        new_status_primitive = PURPLE_STATUS_AWAY;
    } else if (status == "invisible") {
        new_status_primitive = PURPLE_STATUS_INVISIBLE;
    } else if (status == "offline") {
        new_status_primitive = PURPLE_STATUS_OFFLINE;
    }
    return new_status_primitive;
}


vector<string> accounts_iterate(std::function<string (PurpleAccount*)> func,
                                bool only_active)
{
    vector<string> result;
    GList *p_accounts = (only_active ? purple_accounts_get_all_active() :
                         p_accounts = purple_accounts_get_all());
    PurpleAccount *p_acc = nullptr;
    while (p_accounts) {
        p_acc = reinterpret_cast<PurpleAccount*>(p_accounts->data);
        result.push_back(func(p_acc));
        p_accounts = g_list_next(p_accounts);
    }
    return result;
}


bool statuses_iterate_till_true(const PurpleAccount *p_acc,
                                std::function<bool (const PurpleStatus*)> func)
{
    PurplePresence *p_presence = purple_account_get_presence(p_acc);
    GList *p_statuses = purple_presence_get_statuses(p_presence);
    while (p_statuses) {
        PurpleStatus *p_stat = reinterpret_cast<PurpleStatus*>(p_statuses->data);
        if (func(p_stat)) {
            return true;
        }
        p_statuses = g_list_next(p_statuses);
    }
    return false;
}

}
