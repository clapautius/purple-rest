/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef PURPLE_INTERACTION_HPP
#define PURPLE_INTERACTION_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "imbuddy.hpp"

extern "C"
{

#include "plugin.h"
#include "notify.h"

void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix);

};

namespace libpurple
{

void purple_info(const std::string &msg);

/**
 * Helper function to collect all buddies from the purple buddy tree.
 *
 * @param[in] online_only : collect only online buddies
 */
void collect_buddies(PurpleBlistNode *p, std::vector<p_rest::Buddy> &list,
                     bool online_only = false);

/**
 * Helper function to get the group of a buddy.
 * Climbs up into the tree until it finds a group node.
 *
 * @return the name of the group or empty string if no group is found.
 */
std::string buddy_get_group_name(PurpleBlistNode *p);

/**
 * @return The active status of the specified account.
 **/
std::string get_status_for_account(PurpleAccount *p_account);

/**
 * @return The active status of the specified account.
 **/
std::string get_status_for_account(const std::string &account_name,
                                   bool only_active = true);

/**
 * @param[in] debug_on : add debug info about statuses (active, exclusive, etc.)
 **/
std::vector<std::string> get_statuses_for_account(const PurpleAccount *p_account,
                                                  bool debug_on = false);

/**
 * @param[in] debug_on : add debug info about statuses (active, exclusive, etc.)
 **/
std::vector<std::string> get_statuses_for_account(const std::string &account_name,
                                                  bool debug_on = false,
                                                  bool only_active = true);

/**
 * @return a map containing the active status of all the active accounts.
 */
std::map<std::string, std::string> get_status_for_accounts();

/**
 * Set status for all active accounts.
 *
 * @param[in] status: one of: 'available', 'away', 'invisible'. (WIP)
 *
 * @return true if the status is correct, false otherwise. In order to see if the status
 * has been changed for all the accounts, you'll have to use 'get_status_for_accounts'.
 */
bool set_status_for_all_accounts(const std::string &status);

/**
 * @return true if the status has been changed, false otherwise.
 */
bool set_status_for_account(const std::string &account_name, const std::string &status,
                            bool only_active = true);

/**
 * Low-level function for setting status for an account.
 *
 * @return true if the status has been changed, false otherwise.
 */
bool set_status_for_account(const PurpleAccount *p_acc, PurpleStatusPrimitive status);

void reset_idle();

PurpleBuddy* get_buddy_by_name(const std::string &buddy_name);

PurpleAccount* get_account_by_name(const std::string &account_name,
                                   bool only_active = true);

/**
 * @return A name for the purple account (which can be the alias, the username or
 * 'Unknown_account' if none is available.
 */
std::string get_purple_account_name(const PurpleAccount*);

/**
 * @return PURPLE_STATUS_UNSET if no such status.
 */
PurpleStatusPrimitive get_primitive_from_status_string(const std::string &status);

/**
 * Iterate through the accounts and call the FUNC specified for each account.
 * Collect the results in a vector of strings that is returned.
 */
std::vector<std::string> accounts_iterate(std::function<std::string (PurpleAccount*)> func,
                                          bool only_active = true);


/**
 * Iterate through the statuses of an account and call the FUNC specified for each status.
 * Stop when a function return true.
 */
bool statuses_iterate_till_true(const PurpleAccount*,
                                std::function<bool (const PurpleStatus*)> func);

}

#endif
