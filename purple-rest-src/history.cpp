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
std::string History::get_history_list_as_html(uint64_t start_from_id)
{
    std::ostringstream ostr;
    for (auto e : m_message_list) {
        // :fixme: use a different pattern (a writer)
        // :fixme: optimize this - use a different container?
        if (e->get_id() > start_from_id) {
            ostr << "id: " << e->get_id() << "; msg: " << e->get_message()
                 << "<br>" << std::endl;
        }
    }
    purple_info(std::string("history (html): ") + ostr.str());
    return ostr.str();
}


std::string History::get_history_list_as_json(uint64_t start_from_id)
{
    Json::Value msg_list(Json::arrayValue);
    for (auto e : m_message_list) {
        // :fixme: optimize this - use a different container?
        if (e->get_id() > start_from_id) {
            Json::Value msg(Json::objectValue);
            msg["id"] = static_cast<unsigned int>(e->get_id());
            msg["text"] = e->get_message();
            msg_list.append(msg);
        }
    }
    purple_info(std::string("history (json): ") + msg_list.toStyledString());
    return msg_list.toStyledString();
}


uint64_t History::get_new_id()
{
    return m_free_id.fetch_add(1);
}


std::atomic<uint64_t> History::m_free_id(1);

}
