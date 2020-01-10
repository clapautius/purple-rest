/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <memory>
#include <vector>
#include <atomic>
#include <functional>
#include <list>

#include "immessage.hpp"

namespace p_rest
{

class History
{
public:

    void add_im_message(std::shared_ptr<ImMessage> msg);

    std::list<ImMessagePtr>
    get_messages_from_history(std::function<bool(ImMessagePtr&)> expr) const;

    static uint64_t get_new_id();

    static uint64_t get_max_id()
    {
        return m_free_id -1;
    }

    static msg_id_t get_max_my_msg_id()
    {
        return m_max_my_msg_id;
    }

    void clear_history();

private:

    std::vector<ImMessagePtr> m_message_list;

    static std::atomic<msg_id_t> m_free_id;

    static msg_id_t m_max_my_msg_id;
};

};

#endif
