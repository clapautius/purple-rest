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
#include <memory>

#include "plugin.h"
#include "notify.h"

namespace purple
{

class ImMessage
{
public:

    typedef enum {
        kMsgTypeUnknown = 0,
        kMsgTypeIm,
        kMsgTypeChat
    } ImMessageType;

    ImMessage(PurpleAccount *account, const char *msg, uint64_t id, const char *sender, ImMessageType type = kMsgTypeUnknown);

    const std::string &get_text() const;

    const std::string &get_sender() const;

    uint64_t get_id() const
    {
        return m_id;
    }

    ImMessageType get_type() const
    {
        return m_type;
    }

private:

    PurpleAccount *m_account;

    std::string m_message;

    time_t m_recv_time;

    uint64_t m_id;

    std::string m_sender;

    ImMessageType m_type;
};

typedef std::shared_ptr<ImMessage> ImMessagePtr;

};

#endif
