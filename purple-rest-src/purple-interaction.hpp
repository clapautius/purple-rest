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

std::string get_account_status();

std::map<std::string, std::string> get_accounts_status();

bool set_status_for_all_accounts(const std::string &status);

void reset_idle();

PurpleBuddy* get_buddy_by_name(const std::string &buddy_name);

/**
 * @return A name for the purple account (which can be the alias, the username or
 * 'Unknown_account' if none is available.
 */
std::string get_purple_account_name(const PurpleAccount*);

}

#endif
