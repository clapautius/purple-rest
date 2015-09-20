/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef REST_API_HPP
#define REST_API_HPP

extern "C"
{

void perform_rest_request(const char *url, const char *method,
                          char **buf, int *buf_len, char **content_type,
                          int *http_code);

}

#endif
