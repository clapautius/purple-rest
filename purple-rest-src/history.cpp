/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <string.h>
#include <sstream>

#include "history.hpp"
#include "purple-interaction.hpp"

namespace purple
{

void History::add_im_message(std::shared_ptr<ImMessage> msg)
{
    m_message_list.push_back(msg);
}


std::list<ImMessagePtr> History::get_messages_from_history(
  std::function<bool(ImMessagePtr&)> expr) const
{
    std::list<std::shared_ptr<ImMessage>> result;
    for (auto m : m_message_list) {
        if (expr(m)) {
            result.push_back(m);
        }
    }
    return result;
}


uint64_t History::get_new_id()
{
    return m_free_id.fetch_add(1);
}


std::atomic<uint64_t> History::m_free_id(1);

}
