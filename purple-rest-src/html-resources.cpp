/* Purple REST plugin -- Copyright (C) 2019-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <plugin.h>
#include "html-resources.hpp"

HtmlTemplate::HtmlTemplate()
{
    m_message_system_prefix = "ⓘ";
}


void HtmlTemplate::read_config()
{
    const char *p = purple_prefs_get_string("/plugins/core/purple-rest/html-tpl-message-system-prefix");
    if (p) {
        m_message_system_prefix = p;
        p = nullptr;
    }
}


std::string HtmlTemplate::get_message_system_prefix() const
{
    return m_message_system_prefix;
}

/**
 * @todo :fixme: get rid of this global
 */
HtmlTemplate g_html_template;
