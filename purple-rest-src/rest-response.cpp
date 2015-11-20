/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "rest-response.hpp"
#include "html-resources.hpp"

using std::string;

namespace purple
{

RestResponse::~RestResponse()
{
}


HtmlResponse::HtmlResponse()
{
}


HtmlResponse::~HtmlResponse()
{
}


std::string HtmlResponse::get_text()
{
    return m_ostr.str();
}


void HtmlResponse::add_message(std::shared_ptr<ImMessage> &msg)
{
    m_ostr << "<div class=\"message\">"
           << "<span class=\"message-sender\">" << msg->get_sender()
           << "&nbsp;(" << msg->get_short_date_string() << "):"
           << "</span><br>\n"
           << "<span class=\"message-text\">" << msg->get_text() << "</span></div>\n";
}


void HtmlResponse::add_conversation(PurpleConversation *conv, unsigned conv_id)
{
    if (conv) {
        const char *conv_name = purple_conversation_get_title(conv);
        m_ostr << "<span class=\"conversation\" id=\"conv_" << conv_id << "\">"
               << (conv_name ? conv_name : "unknown conversation")
               << "</span>\n";
    }
}


void HtmlResponse::add_generic_param(const std::string &param_name,
                                     unsigned value)
{
    m_ostr << "<span>" << param_name << " = " << value << "</span>\n";
}


void HtmlResponse::add_buddy(const Buddy &buddy)
{
    string status_class = (buddy.is_online() ? "buddy-online" : "buddy-offline");
    m_ostr << "<div class=\"buddy," << status_class << "\">"
           << "<span class=\"buddy-name," << status_class << "\">" << buddy.get_name()
           << "</span></div><br>\n";
}


JsonResponse::JsonResponse()
  : m_msg_list(Json::arrayValue)
{

}


JsonResponse::~JsonResponse()
{
}


std::string JsonResponse::get_text()
{
    return m_msg_list.toStyledString();
}


void JsonResponse::add_message(std::shared_ptr<ImMessage> &msg)
{
    Json::Value new_msg(Json::objectValue);
    new_msg["id"] = static_cast<unsigned int>(msg->get_id());
    new_msg["text"] = msg->get_text();
    new_msg["sender"] = msg->get_sender();
    new_msg["conversation"] = msg->get_conv_id();
    m_msg_list.append(new_msg);
}


void JsonResponse::add_conversation(PurpleConversation *conv, unsigned conv_id)
{
    if (conv) {
        Json::Value new_conv(Json::objectValue);
        const char *conv_name = purple_conversation_get_title(conv);
        new_conv["name"] = (conv_name ? conv_name : "unknown");
        new_conv["id"] = conv_id;
        // :fixme: - rename m_msg_list
        m_msg_list.append(new_conv);
    }
}


void JsonResponse::add_generic_param(const std::string &param_name,
                                     unsigned value)
{
    Json::Value new_param(Json::objectValue);
    new_param[param_name.c_str()] = value;
    // :fixme: - rename m_msg_list
    m_msg_list.append(new_param);
}


void JsonResponse::add_buddy(const Buddy &buddy)
{
    Json::Value new_buddy(Json::objectValue);
    new_buddy["name"] = buddy.get_name();
    new_buddy["group"] = buddy.get_group();
    new_buddy["status"] = buddy.is_online() ? "online" : "offline";
    // :fixme: - add the rest of the elements
    // :fixme: - rename m_msg_list
    // :fixme: - group them by group
    m_msg_list.append(new_buddy);
}


/**
 * @param[in] type : html / json
 * @param[out] response : the newly created response object
 * @param[out] content_type : the HTTP content_type
 * @return true if OK, false on error
 */
bool create_response(const std::string &type, std::unique_ptr<RestResponse> &response,
                     std::string &content_type)
{
    bool rc = true;
    if (type == "json") {
        response.reset(new purple::JsonResponse);
        content_type = "application/json";
    }
    else if (type == "html") {
        response.reset(new purple::HtmlResponse);
        content_type = "text/html";
    }
    else {
        rc = false;
    }
    return rc;
}


}
