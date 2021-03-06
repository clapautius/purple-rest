/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef REST_API_HPP
#define REST_API_HPP

#include "purple-rest.h"

extern "C"
{

void perform_rest_request(const char *url, HttpMethod method,
                          const char *upload_data, size_t upload_data_size,
                          char **buf, int *buf_len, char **content_type,
                          int *http_code);

void init_rest_api();

}

#endif
