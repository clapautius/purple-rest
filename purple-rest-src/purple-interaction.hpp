#ifndef PURPLE_INTERACTION_HPP
#define PURPLE_INTERACTION_HPP

extern "C"
{

#include "plugin.h"

void init_purple_rest_module(PurplePlugin *plugin);

};

void purple_info(const std::string &msg);

#endif
