/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef IMCONVERSATION_HPP
#define IMCONVERSATION_HPP

#include <string>
#include <vector>
#include <atomic>

#include "plugin.h"
#include "notify.h"

namespace p_rest
{

typedef uint64_t conv_id_t;

class ImConversation
{
public:

    ImConversation(PurpleConversation *conv);

    std::string get_name() const;

    PurpleConversation* get_purple_conv() const
    {
        return m_purple_conv;
    }

    conv_id_t id() const
    {
        return m_id;
    }

    bool operator==(const ImConversation &rhs) const
    {
        return (m_id == rhs.id() &&
                m_purple_conv == rhs.m_purple_conv);
    }

    /**
     * Valid conversation objects must have an id different of 0.
     */
    operator bool() const
    {
        return m_id != 0;
    }

private:

    PurpleConversation* m_purple_conv;

    conv_id_t m_id;

    static std::atomic<conv_id_t> m_free_id;
};


class ImConversationsList
{
public:

    ImConversationsList();

    conv_id_t add_conversation(ImConversation conv);

    /**
     * @return the conversation id (which is not the same as the index)
     */
    conv_id_t get_or_add_conversation(PurpleConversation *conv);

    unsigned size() const;

    ImConversation& operator[](unsigned idx);

    ImConversation& operator[](const std::string&);

    /**
     * @return the conversation that matches the ID, or
     * m_null_conv if no such conversation exists.
     */
    ImConversation& get_conversation_by_id(conv_id_t id);

    conv_id_t get_conversation_id(PurpleConversation *conv) const;

    /**
     * Remove the conversation with the id ID from the converation list.
     */
    void remove_conversation(conv_id_t id);

    static ImConversation m_null_conv;

private:

    std::vector<ImConversation> m_conversations;
};

extern ImConversationsList g_conv_list;

}

#endif
