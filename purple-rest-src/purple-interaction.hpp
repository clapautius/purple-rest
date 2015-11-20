/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef PURPLE_INTERACTION_HPP
#define PURPLE_INTERACTION_HPP

#include <string>
#include <vector>

#include "imbuddy.hpp"

extern "C"
{

#include "plugin.h"
#include "notify.h"

void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix);

gboolean timeout_cb(gpointer user_data);

};

namespace libpurple
{

struct SendMsgData
{
    const PurpleConversation *conv;
    std::string msg;
};

extern struct SendMsgData g_send_msg_data;

void purple_info(const std::string &msg);

/**
 * Helper function to collect all buddies from the purple buddy tree.
 */
void collect_buddies(PurpleBlistNode *p, std::vector<purple::Buddy> &list);

/**
 * Helper function to get the group of a buddy.
 * Climbs up into the tree until it finds a group node.
 *
 * @return the name of the group or empty string if no group is found.
 */
std::string buddy_get_group_name(PurpleBlistNode *p);

}

#endif
