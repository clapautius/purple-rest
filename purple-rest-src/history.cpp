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


char* History::get_full_history_list()
{
    std::ostringstream ostr;
    ostr << "<html><body>";
    // :fixme:
    for (unsigned i = 0; i < m_message_list.size(); i++) {
        ostr << m_message_list[i]->get_message() << "<br>";
    }
    ostr << "</body></html>";
    purple_info(std::string("!! HISTORY: %s") + ostr.str());
    return ::strdup(ostr.str().c_str());
}

}
