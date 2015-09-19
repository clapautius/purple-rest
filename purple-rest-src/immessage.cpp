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
