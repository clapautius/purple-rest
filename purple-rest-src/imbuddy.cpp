/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "imbuddy.hpp"

namespace purple
{

Buddy::Buddy(const char* p_name, const char *p_group)
  : m_online(false)
{
    if (p_name) {
        m_name = p_name;
    } else {
        m_name = "<NULL>";
    }
    if (p_group) {
        m_group = p_group;
    }
}

const std::string& Buddy::get_name() const
{
    return m_name;
}


const std::string& Buddy::get_group() const
{
    return m_group;
}


void Buddy::set_group(const char *p)
{
    if (p) {
        m_group = p;
    }
}

}
