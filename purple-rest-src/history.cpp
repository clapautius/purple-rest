/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <string.h>
#include <sstream>

#include<json/writer.h>

#include "history.hpp"
#include "purple-interaction.hpp"

namespace purple
{

void History::add_im_message(std::shared_ptr<ImMessage> msg)
{
    m_message_list.push_back(msg);
}


/**
 * Returns all IM messages separated by '<br>'.
 */
std::string History::get_history_list_as_html()
{
    std::ostringstream ostr;
    for (auto e : m_message_list) {
        ostr << e->get_message() << "<br>";
    }
    purple_info(std::string("history (html): %s") + ostr.str());
    return ostr.str();
}


std::string History::get_history_list_as_json()
{
    Json::Value msg_list(Json::arrayValue);
    msg_list.append("Message list:");
    for (auto e : m_message_list) {
        msg_list.append(e->get_message());
    }
    purple_info(std::string("history (json): %s") + msg_list.toStyledString());
    return msg_list.toStyledString();
}

}
