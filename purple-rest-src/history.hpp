#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <memory>
#include <vector>

#include "immessage.hpp"

namespace purple
{

class History
{
public:

    void add_im_message(std::shared_ptr<ImMessage> msg);

    // :fixme: temporary
    // caller must std::free the returned buffer
    char* get_full_history_list();

private:

    std::vector< std::shared_ptr<ImMessage> > m_message_list;
};

};

#endif
