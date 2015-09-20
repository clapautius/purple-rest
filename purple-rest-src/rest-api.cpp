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
#include "rest-response.hpp"

using std::string;
using std::vector;

extern purple::History g_msg_history;


/**
 * @return HTTP code to be sent back to user.
 */
static int get_messages_request(const vector<std::string> &request, string &s,
                                 char **content_type)
{
    // analyze request params
    // req[3] = 'start_from', req[4] = id
    uint64_t start_from_id = 0;
    std::unique_ptr<purple::RestResponse> response;
    if (request.size() >= 5) {
        if (request[3] == "start_from") {
            // :fixme: - check for errors
            start_from_id = strtol(request[4].c_str(), NULL, 10);
        }
    }
    if (request[1] == "json") {
        response.reset(new purple::JsonResponse);
        *content_type = strdup("application/json");
    }
    else if (request[1] == "html") {
        response.reset(new purple::HtmlResponse);
        *content_type = strdup("text/html");
    }
    else {
        return 400;
    }
    std::list<std::shared_ptr<purple::ImMessage>> msg_list =
      g_msg_history.get_messages_from_history(
        [=] (std::shared_ptr<purple::ImMessage> &elt) -> bool
        {
            return (elt->get_id() > start_from_id);
        });
    for (auto e : msg_list) {
        response->add_message(e);
    }
    s = response->get_text();
    return 200;
}


void perform_rest_request(const char *url, const char *method,
                          char **buf, int *buf_len, char **content_type, int *http_code)
{
    std::string s;
    purple_info(std::string("Got new request: ") + url);

    // tokenize url
    std::vector<std::string> request;

    // strtok wants to modify the string, so we make a copy
    char *url_to_be_tokenized = strdup(url);
    char *p = strtok(url_to_be_tokenized, "/");
    while (p) {
        request.push_back(p);
        p = strtok(NULL, "/");
    }
    free(url_to_be_tokenized);

    if (request.size() >= 3) {
        // first element is version - ignore it for now - :fixme:

        purple_info(std::string("Output type: ") + request[1]);

        if (request[2] == "messages") {
            *http_code = get_messages_request(request, s, content_type);
        } else {
            *http_code = 400;
        }
    } else {
        *http_code = 400;
    }

    if (s.size() > 0) {
        *buf = (char*)malloc(s.size() + 1);
        strncpy(*buf, s.c_str(), s.size() + 1);
        *buf_len = s.size();
    } else {
        *buf = NULL;
        *buf_len = 0;
        *content_type = NULL;
    }
}
