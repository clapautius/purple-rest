/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef PURPLE_REST_H
#define PURPLE_REST_H

#define PLUGIN_ID "core-purple_rest"

// uncomment this for extra logging and more details in the HTTP responses
#define PURPLE_REST_DEBUG

typedef enum
{
    kHttpMethodUndefined,
    kHttpMethodPost,
    kHttpMethodGet,
    kHttpMethodDelete,
    kHttpMethodPut
} HttpMethod;

#endif
