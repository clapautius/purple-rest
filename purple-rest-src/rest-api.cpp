/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include <string.h>
#include <string>
#include <algorithm>
#include <map>
#include <functional>

#include <json/json.h>

#include "rest-api.hpp"
#include "history.hpp"
#include "purple-interaction.hpp"
#include "rest-response.hpp"

using std::string;
using std::vector;
using libpurple::purple_info;

extern p_rest::History g_msg_history;
extern std::string g_url_prefix;

const int kFormatIdx = 1;


/**
 * Helper function to convert a string to a uint64.
 *
 * @return true if OK, false on error.
 */
static bool str_to_uint64_t(const char*str, uint64_t &num)
{
    char *p = NULL;
    long int ret = strtol(str, &p, 10);
    if (ret == 0 && p == str) {
        return false;
    } else if (ret < 0) {
        return false;
    } else {
        num = ret;
        return true;
    }
}


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
    using p_rest::ImMessage;
    const int kStartFromIdIdx = 4;
    const int kMsgTypeIdx = 3;
    // analyze request params
    uint64_t start_from_id = 0;
    ImMessage::ImMessageType filter = ImMessage::kMsgTypeUnknown; // == no filter
    std::unique_ptr<p_rest::RestResponse> response;
    if (request.size() > kMsgTypeIdx) {
        if (request[kMsgTypeIdx] == "im") {
            filter = ImMessage::kMsgTypeIm;
        } else if (request[kMsgTypeIdx] == "chat") {
            filter = ImMessage::kMsgTypeChat;
        }
    }
    if (request.size() > kStartFromIdIdx + 1) {
        if (request[kStartFromIdIdx] == "start_from") {
            if (!str_to_uint64_t(request[kStartFromIdIdx + 1].c_str(), start_from_id)) {
                goto error;
            }
        }
    }
    if (create_response(request[1], response, content_type)) {
        auto msg_list = g_msg_history.get_messages_from_history(
          [=] (p_rest::ImMessagePtr &elt) -> bool
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
error:
    return 400;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/my-messages/
 * /v/<format>/my-messages/start_from/<msg_id>
 */
static int get_my_messages_request(const vector<string> &request, string &response_str,
                                   string &content_type)
{
    using p_rest::ImMessage;
    const int kStartFromIdIdx = 3;
    // analyze request params
    uint64_t start_from_id = 0;
    std::unique_ptr<p_rest::RestResponse> response;
    if (request.size() > kStartFromIdIdx + 1) {
        if (request[kStartFromIdIdx] == "start_from") {
            if (!str_to_uint64_t(request[kStartFromIdIdx + 1].c_str(), start_from_id)) {
                goto error;
            }
        }
    }
    if (p_rest::create_response(request[1], response, content_type)) {
        auto msg_list = g_msg_history.get_messages_from_history(
          [=] (p_rest::ImMessagePtr &elt) -> bool
          {
              return (elt->get_id() > start_from_id &&
                      (ImMessage::kMsgTypeIm == elt->get_type() ||
                       ImMessage::kMsgTypeChatAcc == elt->get_type()));
          });
        for (auto &e : msg_list) {
            response->add_message(e);
        }
        response_str = response->get_text();
        return 200;
    }

error:
    return 400;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * :fixme: maybe rename this to 'state', to avoid confusion with buddy / account
 * status?
 *
 * Requests summary:
 * /v/<format>/status/max_msg_id
 * /v/<format>/status/max_my_msg_id
 * /v/<format>/status/account-status
 */
static int get_status_request(const vector<string> &request, string &response_str,
                              string &content_type)
{
    std::unique_ptr<p_rest::RestResponse> response;
    int param_idx = 3;
    if (request.size() <= 3) {
        goto error;
    } else {
        if (!p_rest::create_response(request[1], response, content_type)) {
            goto error;
        }
    }
    if (request[param_idx] == "max_msg_id") {
        response->add_generic_param("max_msg_id", g_msg_history.get_max_id());
    } else if (request[param_idx] == "max_my_msg_id") {
        response->add_generic_param("max_my_msg_id", g_msg_history.get_max_my_msg_id());
    } else if (request[param_idx] == "account-status") {
        response->add_generic_param("status", libpurple::get_account_status().c_str());
    } else {
        goto error;
    }
    response_str = response->get_text();
    return 200;
error:
    return 400;
}



/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/status/max_msg_id
 * /v/<format>/status/max_my_msg_id
 */
static int get_cmd_request(const vector<string> &request, string &response_str,
                           string &content_type)
{
    std::unique_ptr<p_rest::RestResponse> response;
    int cmd_idx = 3;
    if (request.size() <= 3) {
        goto error;
    } else {
        if (!p_rest::create_response(request[1], response, content_type)) {
            goto error;
        }
    }
    if (request[cmd_idx] == "clear_history") {
        g_msg_history.clear_history();
    } else {
        goto error;
    }
    response_str = response->get_text();
    return 200;
error:
    return 400;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/conv-messages/<id>/start_from/<msg_id>
 */
static int get_conv_messages_request(const vector<string> &request, string &response_str,
                                     string &content_type)
{
    std::unique_ptr<p_rest::RestResponse> response;
    const int kConvIdIdx = 3;
    uint64_t conv_id = 0;
    if (request.size() > kConvIdIdx) {
        if (request[1] == "json") {
            response.reset(new p_rest::JsonResponse);
            content_type = "application/json";
        }
        else if (request[1] == "html") {
            response.reset(new p_rest::HtmlResponse);
            content_type = "text/html";
        }
        else {
            return 400;
        }
        if (!str_to_uint64_t(request[kConvIdIdx].c_str(), conv_id)) {
            goto error;
        }
        const int kStartFromIdIdx = 4;
        uint64_t start_from_id = 0;
        if (request.size() > kStartFromIdIdx + 1) { // we might have 'start_from'
            if (request[kStartFromIdIdx] == "start_from") {
                if (!str_to_uint64_t(request[kStartFromIdIdx + 1].c_str(), start_from_id)) {
                    goto error;
                }
            }
        }
        auto msg_list = g_msg_history.get_messages_from_history(
          [=] (p_rest::ImMessagePtr &elt) -> bool
          {
              return (conv_id == elt->get_conv_id() &&
                      elt->get_id() > start_from_id);
          });
        for (auto &e : msg_list) {
            response->add_message(e);
        }
    }

    response_str = response->get_text();
    return 200;
error:
    return 400;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/conversations/all
 */
static int get_conversations_request(const vector<string> &request, string &response_str,
                                     string &content_type)
{
    using p_rest::g_conv_list;
    std::unique_ptr<p_rest::RestResponse> response;
    if (request.size() > 3) {
        if (request[1] == "json") {
            response.reset(new p_rest::JsonResponse);
            content_type = "application/json";
        }
        else if (request[1] == "html") {
            response.reset(new p_rest::HtmlResponse);
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
                response->add_conversation(cv, g_conv_list.get_or_add_conversation(cv));
                ptr = g_list_next(ptr);
            }
        } else {
            goto error;
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
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/conversations/<id>
 */
static int delete_conversations_request(const vector<string> &request,
                                        string &response_str, string &content_type)
{
    using p_rest::ImConversation;
    using p_rest::g_conv_list;
    int err_code = 500;
    if (request.size() > 3) {
        // we don't care about format atm, we don't send anything back
        // :fixme: we should send a human readable error message
        p_rest::conv_id_t conv_id = 0;
        if (str_to_uint64_t(request[3].c_str(), conv_id)) {
            ImConversation &conv = g_conv_list.get_conversation_by_id(conv_id);
            if (conv == g_conv_list.m_null_conv) {
                purple_info("No such conversation");
                err_code = 404;
                goto error;
            } else {
                PurpleConversation *p_conv = conv.get_purple_conv();
                g_conv_list.remove_conversation(conv_id);
                if (p_conv) {
                    purple_conversation_destroy(p_conv);
                }
            }
        } else {
            err_code = 400;
            goto error;
        }
    } else {
        goto error;
    }
    return 200;
error:
    return err_code;
}


/**
 * @param[in] request : vector containing URL components (elements separated by '/').
 * @param[out] response_str : response to be sent back.
 * @param[out] content_type : string describing the MIME type of the response.
 *
 * @return HTTP code to be sent back to user.
 *
 * Requests summary:
 * /v/<format>/buddies/<all | online>
 */
static int get_buddies_request(const vector<string> &request, string &response_str,
                               string &content_type)
{
    using p_rest::Buddy;
    std::unique_ptr<p_rest::RestResponse> response;
    const int kFilterIdx = 3;
    if (request.size() > 3) {
        if (request[kFormatIdx] == "json") {
            response.reset(new p_rest::JsonResponse);
            content_type = "application/json";
        }
        else if (request[kFormatIdx] == "html") {
            response.reset(new p_rest::HtmlResponse);
            content_type = "text/html";
        }
        else {
            return 400;
        }
        if (request[kFilterIdx] == "all" || request[kFilterIdx] == "online") {
            std::vector<Buddy> buddy_list;
            libpurple::collect_buddies(purple_get_blist()->root, buddy_list,
                                       (request[kFilterIdx] == "online"));
            // sort by group and name
            std::sort(buddy_list.begin(), buddy_list.end(),
                      [] (const Buddy &b1, const Buddy &b2) -> bool {
                          return (b1.get_group() < b2.get_group()) ||
                            (b1.get_group() == b2.get_group() &&
                             b1.get_name() < b2.get_name());
                      });
            for (auto &b : buddy_list) {
                response->add_buddy(b);
            }
        } else {
            goto error;
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
 * /v/<format>/conv-messages/<id>
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
              p_rest::g_conv_list[conv_id].get_purple_conv();
            // set account as 'available'
            libpurple::reset_idle();
            // set callback data
            libpurple::g_send_msg_data.conv = conv;
            libpurple::g_send_msg_data.msg = string(upload_data, upload_data_size);
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
    purple_info(std::string("Prefix is: ") + g_url_prefix);

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
            purple_info("Unable to find prefix in request, aborting.");
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

    // :debug:
    purple_info("Request elements");
    int idx = 0;
    std::ostringstream ostr;
    for (auto &elt : request) {
        ostr << idx++ << " : " << elt.c_str() << std::endl;
    }
    purple_info(ostr.str());

    using RequestMap = std::map<string, std::function<int(const vector<string>&,
                                                          string&, string &)>>;

    RequestMap get_actions = { { "messages", get_messages_request },
                               { "conversations", get_conversations_request },
                               { "my-messages", get_my_messages_request },
                               { "status", get_status_request },
                               { "conv-messages", get_conv_messages_request },
                               { "cmd", get_cmd_request },
                               { "buddies", get_buddies_request }
    };

    RequestMap delete_actions = { { "conversations", delete_conversations_request } };

    if (kHttpMethodPost == method) {
        *http_code = post_messages_request(request, upload_data, upload_data_size,
                                           response, content_type_str);
        return;
    } else if (kHttpMethodGet == method || kHttpMethodDelete == method) {
        if (request.size() >= 3) {
            // first element is version - ignore it for now

            purple_info(std::string("Output type: ") + request[1]);
            if (kHttpMethodGet == method) {
                if (get_actions.find(request[2]) != get_actions.end()) {
                    *http_code = get_actions[request[2]](request, response,
                                                         content_type_str);
                } else {
                    *http_code = 400;
                }
            } else if (kHttpMethodDelete == method) {
                if (delete_actions.find(request[2]) != delete_actions.end()) {
                    *http_code = delete_actions[request[2]](request, response,
                                                            content_type_str);
                } else {
                    *http_code = 400;
                }
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
