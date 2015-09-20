/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef IMMESSAGE_HPP
#define IMMESSAGE_HPP

#include <time.h>
#include <string>
#include <stdint.h>

#include "plugin.h"
#include "notify.h"

namespace purple
{

class ImMessage
{
public:
    ImMessage(PurpleAccount *account, const char *msg, uint64_t id);

    const std::string &get_message() const;

    uint64_t get_id() const
    {
        return m_id;
    }

private:

    PurpleAccount *m_account;

    std::string m_message;

    time_t m_recv_time;

    uint64_t m_id;
};

};

#endif
