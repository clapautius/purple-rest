#ifndef IMMESSAGE_HPP
#define IMMESSAGE_HPP

#include <time.h>
#include <string>

#include "plugin.h"
#include "notify.h"

namespace purple
{

class ImMessage
{
public:
    ImMessage(PurpleAccount *account, const char *msg);

    const std::string &get_message() const;
private:

    PurpleAccount *m_account;

    std::string m_message;

    time_t m_recv_time;
};

};

#endif
