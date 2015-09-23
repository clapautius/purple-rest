/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef PURPLE_INTERACTION_HPP
#define PURPLE_INTERACTION_HPP

#include <string>

extern "C"
{

#include "plugin.h"

void init_purple_rest_module(PurplePlugin *plugin, const char *url_prefix);

};

void purple_info(const std::string &msg);

#endif
