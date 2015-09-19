/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "immessage.hpp"

namespace purple
{

ImMessage::ImMessage(PurpleAccount *account, const char *msg)
{
    m_account = account;
    m_message = msg;
    m_recv_time = time(NULL);
}


const std::string &ImMessage::get_message() const
{
    return m_message;
}

}
