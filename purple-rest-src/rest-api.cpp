/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <string.h>
#include <string>

#include <json/json.h>

#include "rest-api.hpp"
#include "history.hpp"
#include "purple-interaction.hpp"

extern purple::History g_msg_history;


namespace
{

/**
 * :todo:
 */
std::string html_display_homepage()
{
    return "<body>Not ready yet</body>";
}

};


void perform_rest_request(const char *url, const char *method,
                          char **buf, int *buf_len, char **content_type)
{
    std::string s;
    purple_info(std::string("Got new request: ") + url);

    // tokenize url
    std::vector<std::string> request_elements;

    // strtok wants to modify the string, so we make a copy
    char *url_to_be_tokenized = strdup(url);
    char *p = strtok(url_to_be_tokenized, "/");
    while (p) {
        request_elements.push_back(p);
        p = strtok(NULL, "/");
    }
    free(url_to_be_tokenized);

    if (request_elements.size() >= 2) {
        // first element is version - ignore it for now - :fixme:

        purple_info(std::string("Output type: ") + request_elements[1]);
        if (request_elements[1] == "json") {
             s = g_msg_history.get_history_list_as_json();
             *content_type = strdup("text/json");
        }
        else if (request_elements[1] == "html") {
            s = g_msg_history.get_history_list_as_html();
            *content_type = strdup("text/html");
        }
        else {
            s = html_display_homepage();
            *content_type = strdup("text/html");
        }
    }
    // :fixme: - generate error in case of invalid request

    *buf = (char*)malloc(s.size() + 1);
    strncpy(*buf, s.c_str(), s.size() + 1);
    *buf_len = s.size();
}
