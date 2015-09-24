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
#include "imconversation.hpp"

using std::string;
using std::vector;
using purple::ImMessage;

extern purple::History g_msg_history;
extern std::string g_url_prefix;

/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/messages/
 * /v/<format>/messages/<all | im | chat>/start_from/<msg_id>
 */
static int get_messages_request(const vector<string> &request, string &response_str,
                                 string &content_type)
{
    const int kStartFromIdIdx = 4;
    const int kMsgTypeIdx = 3;
    // analyze request params
    uint64_t start_from_id = 0;
    ImMessage::ImMessageType filter = ImMessage::kMsgTypeUnknown; // == no filter
    std::unique_ptr<purple::RestResponse> response;
    if (request.size() > kMsgTypeIdx) {
        if (request[kMsgTypeIdx] == "im") {
            filter = ImMessage::kMsgTypeIm;
        } else if (request[kMsgTypeIdx] == "chat") {
            filter = ImMessage::kMsgTypeChat;
        }
    }
    if (request.size() > kStartFromIdIdx + 1) {
        if (request[kStartFromIdIdx] == "start_from") {
            // :fixme: - check for errors
            start_from_id = strtol(request[kStartFromIdIdx + 1].c_str(), NULL, 10);
        }
    }
    if (request[1] == "json") {
        response.reset(new purple::JsonResponse);
        content_type = "application/json";
    }
    else if (request[1] == "html") {
        response.reset(new purple::HtmlResponse);
        content_type = "text/html";
    }
    else {
        return 400;
    }
    auto msg_list = g_msg_history.get_messages_from_history(
      [=] (purple::ImMessagePtr &elt) -> bool
      {
          return (elt->get_id() > start_from_id &&
                  (filter == ImMessage::kMsgTypeUnknown || filter == elt->get_type()));
      });
    for (auto &e : msg_list) {
        response->add_message(e);
    }
    response_str = response->get_text();
    return 200;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/conversations/all :fixme: - replace with 'conversations_list' or smth.
 * /v/<format>/conversations/<id>
 */
static int get_conversations_request(const vector<string> &request, string &response_str,
                                     string &content_type)
{
    std::unique_ptr<purple::RestResponse> response;
    unsigned int conv_id = 0;
    if (request.size() > 3) {
        if (request[1] == "json") {
            response.reset(new purple::JsonResponse);
            content_type = "application/json";
        }
        else if (request[1] == "html") {
            response.reset(new purple::HtmlResponse);
            content_type = "text/html";
        }
        else {
            return 400;
        }
        if (request[3] == "all") {
            GList *conversations_list = purple_get_conversations();
            GList *ptr = g_list_first(conversations_list);
            while (ptr) {
                PurpleConversation *cv = reinterpret_cast<PurpleConversation*>(ptr->data);
                std::ostringstream debug_text;
                string name = purple_conversation_get_name(cv);
                string title = purple_conversation_get_title(cv);
                debug_text << "conversation: " << name << ", " << title;
                purple_info(debug_text.str());
                response->add_conversation(cv);
                ptr = g_list_next(ptr);
            }
        } else {
            // :fixme: - check for errors
            conv_id = strtol(request[3].c_str(), NULL, 10);
            auto msg_list = g_msg_history.get_messages_from_history(
              [=] (purple::ImMessagePtr &elt) -> bool
              {
                  return (conv_id == elt->get_conv_id());
              });
            for (auto &e : msg_list) {
                response->add_message(e);
            }
        }
    } else {
        goto error;
    }

    response_str = response->get_text();
    return 200;
error:
    return 400;
}


/**
 * Requests summary:
 * /v/<format>/conversations/<id>
 */
static int post_messages_request(const vector<string> &request,
                                 const char *upload_data, size_t upload_data_size,
                                 string &response_str, string &content_type)
{
    std::ostringstream dbg;
    dbg << "POST request: upload_data=" << upload_data << ", upload_size="
        << upload_data_size;
    purple_info(dbg.str());
    if (request.size() > 3) {
        // :fixme: - check for errors
        unsigned conv_id = strtol(request[3].c_str(), NULL, 10);
        if (conv_id) {
            // :fixme: - check valid data
            const PurpleConversation *conv =
              purple::g_conv_list[conv_id].get_purple_conv();
            g_send_msg_data.conv = conv;
            g_send_msg_data.msg = string(upload_data, upload_data_size);
            purple_timeout_add(100, timeout_cb, NULL);
            response_str = "OK";
            content_type = "text/html";
        }
    } else {
        goto error;
    }
    return 200;

error:
    return 400;
}


/**
 * Function called when the plugin gets a new REST request.
 *
 * @param[in] url : the full URL of the REST request.
 * @param[in] method : HTTP method.
 * @param[out] buf : the response to be sent back (allocated using malloc). The caller
 *   must free it.
 * @param[out] buf_len : the size of BUF.
 * @param[out] content_type : content type of the response, allocated using strdup. Caller
 *   must free it.
 * @param[out] http_code : the HTTP answer code.
 */
void perform_rest_request(const char *url, HttpMethod method,
                          const char *upload_data, size_t upload_data_size,
                          char **buf, int *buf_len, char **content_type, int *http_code)
{
    string response, content_type_str;
    purple_info(string("Got new request: ") + url);

    // basic checks
    if (NULL == url) {
        return;
    }

    std::string url_str(url);
    if (!g_url_prefix.empty()) {
        if (url_str.compare(0, g_url_prefix.size(), g_url_prefix) == 0) {
            url_str = url_str.substr(g_url_prefix.size());
            purple_info(std::string("Request after removing prefix: ") + url_str);
        } else {
            // something is fishy - no prefix - abort
            *http_code = 400;
            *buf = NULL;
            *buf_len = 0;
            *content_type = NULL;
            return;
        }
    }

    // tokenize url
    std::vector<std::string> request;

    // strtok wants to modify the string, so we make a copy
    char *url_to_be_tokenized = strdup(url_str.c_str());
    char *p = strtok(url_to_be_tokenized, "/");
    while (p) {
        request.push_back(p);
        p = strtok(NULL, "/");
    }
    free(url_to_be_tokenized);

    if (kHttpMethodPost == method) {
        *http_code = post_messages_request(request, upload_data, upload_data_size,
                                           response, content_type_str);
        return;
    } else if (kHttpMethodGet == method) {
        if (request.size() >= 3) {
            // first element is version - ignore it for now

            purple_info(std::string("Output type: ") + request[1]);
            if (request[2] == "messages") {
                *http_code = get_messages_request(request, response, content_type_str);
            } else if (request[2] == "conversations") {
                *http_code = get_conversations_request(request, response,
                                                       content_type_str);
            } else {
                *http_code = 400;
            }
        } else {
            *http_code = 400;
        }

        if (response.size() > 0) {
            *buf = (char*)malloc(response.size() + 1);
            strncpy(*buf, response.c_str(), response.size() + 1);
            *buf_len = response.size();
            *content_type = strdup(content_type_str.c_str());
        } else {
            *buf = NULL;
            *buf_len = 0;
            *content_type = NULL;
        }
    } else {
        // unknown method
        goto error;
    }

    return;

error:
    *buf = NULL;
    *buf_len = 0;
    *http_code = 400;
    purple_info("Error processing request");
    return;
}
