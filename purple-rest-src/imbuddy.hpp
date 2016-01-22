/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef IMBUDDY_HPP
#define IMBUDDY_HPP

#include <string>

namespace p_rest
{

class Buddy
{
public:

    Buddy(const char* p_name, const char *p_group = NULL);

    const std::string& get_name() const;

    const std::string& get_group() const;

    void set_group(const char *p);

    bool is_online() const
    {
        return m_online;
    }

    void set_online_status(bool is_online)
    {
        m_online = is_online;
    }

private:

    /// local alias of the buddy (set by user)
    std::string m_name;

    std::string m_group;

    bool m_online;
};

}

#endif
