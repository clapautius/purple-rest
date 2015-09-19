/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

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

    std::string get_history_list_as_html();

    std::string get_history_list_as_json();

private:

    std::vector< std::shared_ptr<ImMessage> > m_message_list;
};

};

#endif
