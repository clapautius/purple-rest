/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "immessage.hpp"

namespace p_rest
{

ImMessage::ImMessage(PurpleAccount *account, const char *msg, uint64_t id,
                     const char *sender, unsigned conversation_id,
                     ImMessageType type, int purple_flags)
  : m_account(account), m_message(msg), m_id(id), m_sender(sender), m_type(type),
    m_conversation_id(conversation_id), m_direction(kMsgDirUndefined)
{
    m_recv_time = time(NULL);
    if (purple_flags & PURPLE_MESSAGE_SEND) {
        m_direction = kMsgDirOutgoing;
    }
    if (purple_flags & PURPLE_MESSAGE_RECV) {
        m_direction = kMsgDirIncoming;
    }
}


const std::string &ImMessage::get_text() const
{
    return m_message;
}

const std::string &ImMessage::get_sender() const
{
    return m_sender;
}


/**
 * Return recv. time as "HH:MM".
 */
std::string ImMessage::get_short_date_string() const
{
    struct tm *recv_time = localtime(&m_recv_time);
    char str[6] = { 0 };
    strftime(str, sizeof(str), "%H:%M", recv_time);
    return std::string(str);
}

}
