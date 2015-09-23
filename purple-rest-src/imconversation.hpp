/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef IMCONVERSATION_HPP
#define IMCONVERSATION_HPP

#include <string>
#include <vector>

#include "plugin.h"
#include "notify.h"

namespace purple
{

class ImConversation
{
public:

    ImConversation(PurpleConversation *conv);

    std::string get_name() const;

    const PurpleConversation* get_purple_conv() const
    {
        return m_purple_conv;
    }

private:

    PurpleConversation* m_purple_conv;

};


class ImConversationsList
{
public:

    ImConversationsList();

    unsigned add_conversation(ImConversation conv);

    unsigned get_or_add_conversation(PurpleConversation *conv);

    unsigned size() const;

    ImConversation& operator[](unsigned idx);

    ImConversation& operator[](const std::string&);

    static ImConversation m_null_conv;

    unsigned get_conversation_id(PurpleConversation *conv) const;

private:

    std::vector<ImConversation> m_conversations;
};

extern ImConversationsList g_conv_list;

}

#endif
