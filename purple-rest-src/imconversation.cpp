/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "imconversation.hpp"

using std::string;
using std::vector;

namespace p_rest
{

ImConversation::ImConversation(PurpleConversation *conv)
  : m_purple_conv(conv)
{
    if (conv) {
        m_id = m_free_id.fetch_add(1);
    } else {
        m_id = 0;
    }
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


std::atomic<conv_id_t> ImConversation::m_free_id(1);


conv_id_t ImConversationsList::add_conversation(ImConversation conv)
{
    m_conversations.push_back(conv);
    return conv.id();
}



conv_id_t ImConversationsList::get_or_add_conversation(PurpleConversation *conv)
{
    conv_id_t id = get_conversation_id(conv);
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


conv_id_t ImConversationsList::get_conversation_id(PurpleConversation *conv) const
{
    for (unsigned int i = 0; i < m_conversations.size(); i++) {
        if (m_conversations[i].get_purple_conv() == conv) {
            return m_conversations[i].id();
        }
    }
    return 0;
}


ImConversation& ImConversationsList::get_conversation_by_id(conv_id_t id)
{
    for (auto &conv : m_conversations) {
        if (conv.id() == id) {
            return conv;
        }
    }
    return m_null_conv;
}


void ImConversationsList::remove_conversation(conv_id_t id)
{
    for (unsigned i = 0; i < m_conversations.size(); i++) {
        if (m_conversations[i].id() == id) {
            m_conversations.erase(m_conversations.begin() + i);
            break;
        }
    }
}


ImConversation ImConversationsList::m_null_conv(nullptr);

ImConversationsList g_conv_list;

}
