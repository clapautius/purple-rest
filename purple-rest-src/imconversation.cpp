/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "imconversation.hpp"

using std::string;
using std::vector;

namespace purple
{

ImConversation::ImConversation(PurpleConversation *conv)
  : m_purple_conv(conv)
{


}


string ImConversation::get_name() const
{
    string ret;
    const char *p = NULL;
    if (m_purple_conv && (p = purple_conversation_get_name(m_purple_conv))) {
        ret = p;
    }
    return ret;
}


unsigned ImConversationsList::add_conversation(ImConversation conv)
{
    m_conversations.push_back(conv);
    return m_conversations.size() - 1;
}



unsigned ImConversationsList::get_or_add_conversation(PurpleConversation *conv)
{
    unsigned id = get_conversation_id(conv);
    if (!id) {
        id = add_conversation(ImConversation(conv));
    }
    return id;
}


unsigned ImConversationsList::size() const
{
    return m_conversations.size();
}


ImConversationsList::ImConversationsList()
{
    // first one is an empty conversation (index 0 is reserved)
    // :fixme: - why 2 copies? - get rid of the static member
    m_conversations.push_back(m_null_conv);
}


ImConversation& ImConversationsList::operator[](unsigned idx)
{
    return m_conversations[idx];
}


ImConversation& ImConversationsList::operator[](const std::string &conv_name)
{
    // :fixme: - optimize this
    for (auto &c : m_conversations) {
        if (c.get_name() == conv_name) {
            return c;
        }
    }
    return m_null_conv;
}



unsigned ImConversationsList::get_conversation_id(PurpleConversation *conv) const
{
    for (unsigned int i = 0; i < m_conversations.size(); i++) {
        if (m_conversations[i].get_purple_conv() == conv) {
            return i;
        }
    }
    return 0;
}


ImConversation ImConversationsList::m_null_conv(NULL);

ImConversationsList g_conv_list;

}
