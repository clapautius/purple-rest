/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "immessage.hpp"

namespace purple
{

ImMessage::ImMessage(PurpleAccount *account, const char *msg, uint64_t id,
                     const char *sender, unsigned conversation_id,
                     ImMessageType type)
  : m_account(account), m_message(msg), m_id(id), m_sender(sender), m_type(type),
    m_conversation_id(conversation_id)
{
    m_recv_time = time(NULL);
}


const std::string &ImMessage::get_text() const
{
    return m_message;
}

const std::string &ImMessage::get_sender() const
{
    return m_sender;
}

}
