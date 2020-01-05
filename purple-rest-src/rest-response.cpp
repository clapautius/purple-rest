/* Purple REST plugin -- Copyright (C) 2015-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "rest-response.hpp"
#include "html-resources.hpp"

using std::string;

namespace p_rest
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
    if (msg->get_type() == ImMessage::kMsgTypeSystem) {
        m_ostr << "<div class=\"message-system\">"
               << "<span class=\"message-system-prefix\">" << g_html_template.get_message_system_prefix() << "</span>"
               << msg->get_short_date_string() << "&nbsp;:&nbsp;"
               << msg->get_text() << "</div>\n";
    } else {
        if (msg->get_direction() == ImMessage::kMsgDirIncoming) {
            m_ostr << "<div class=\"message dir-incoming\">";
        } else if (msg->get_direction() == ImMessage::kMsgDirOutgoing) {
            m_ostr << "<div class=\"message dir-outgoing\">";
        } else {
            m_ostr << "<div class=\"message\">";
        }
        m_ostr << "<span class=\"message-sender\">" << msg->get_sender()
               << "&nbsp;(" << msg->get_short_date_string() << ")"
               << "</span><br>\n"
               << "<span class=\"message-text\">" << msg->get_text() << "</span></div>\n";
    }
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


void HtmlResponse::add_generic_param(const std::string &param_name, const char* p_str)
{
    m_ostr << "<span>" << param_name << " = " <<
      (p_str ? p_str : "NULL") << "</span>\n";
}


void HtmlResponse::add_buddy(const Buddy &buddy)
{
    string status_class = (buddy.is_online() ? "buddy-online" : "buddy-offline");
    m_ostr << "<div class=\"buddy " << status_class << "\">"
           << "<span class=\"buddy-name " << status_class << "\">" << buddy.get_name()
           << "</span></div><br>\n";
}


void HtmlResponse::add_account(const PurpleAccount *account)
{
    if (account) {
        if (account->alias) {
            m_ostr << "<div class=\"account\"><span class=\"account\">"
                   << account->alias << "</span></div><br/>\n";
        } else if (account->username) {
            m_ostr << "<div class=\"account\"><span class=\"account\">"
                   << account->username << "</span></div><br/>\n";
        } else {
            m_ostr << "<div class=\"account\"><span class=\"account\">"
                   << "Unknown" << "</span></div><br/>\n";
        }
    }
}


JsonResponse::JsonResponse()
  : m_response(Json::arrayValue)
{

}


JsonResponse::~JsonResponse()
{
}


std::string JsonResponse::get_text()
{
    return m_response.toStyledString();
}


void JsonResponse::add_message(std::shared_ptr<ImMessage> &msg)
{
    Json::Value new_msg(Json::objectValue);
    new_msg["id"] = static_cast<unsigned int>(msg->get_id());
    new_msg["text"] = msg->get_text();
    new_msg["sender"] = msg->get_sender();
    new_msg["conversation"] = msg->get_conv_id();
    string flags;
    if (msg->get_type() == ImMessage::kMsgTypeSystem) {
        flags += "system:";
    }
    if (msg->get_direction() == ImMessage::kMsgDirIncoming) {
        flags += "dir-incoming:";
    }
    if (msg->get_direction() == ImMessage::kMsgDirOutgoing) {
        flags += "dir-outgoing:";
    }
    new_msg["flags"] = flags;
    m_response.append(new_msg);
}


void JsonResponse::add_conversation(PurpleConversation *conv, unsigned conv_id)
{
    if (conv) {
        Json::Value new_conv(Json::objectValue);
        const char *conv_name = purple_conversation_get_title(conv);
        new_conv["name"] = (conv_name ? conv_name : "unknown");
        new_conv["id"] = conv_id;
        m_response.append(new_conv);
    }
}


void JsonResponse::add_generic_param(const std::string &param_name, unsigned value)
{
    add_generic_param_generic_value(param_name, value);
}


void JsonResponse::add_generic_param(const std::string &param_name, const char* p_str)
{
    add_generic_param_generic_value(param_name, (p_str ? p_str : "NULL"));
}


void JsonResponse::add_buddy(const Buddy &buddy)
{
    Json::Value new_buddy(Json::objectValue);
    new_buddy["name"] = buddy.get_name();
    new_buddy["group"] = buddy.get_group();
    new_buddy["status"] = buddy.is_online() ? "online" : "offline";
    // :fixme: - add the rest of the elements
    // :fixme: - group them by group
    m_response.append(new_buddy);
}


void JsonResponse::add_account(const PurpleAccount *account)
{
    Json::Value new_account(Json::objectValue);
    new_account["username"] = (account->username ? account->username : "None");
    new_account["alias"] = (account->alias ? account->alias : "None");
    new_account["protocol_id"] = (account->protocol_id ? account->protocol_id : "None");
    m_response.append(new_account);
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
        response.reset(new p_rest::JsonResponse);
        content_type = "application/json";
    }
    else if (type == "html") {
        response.reset(new p_rest::HtmlResponse);
        content_type = "text/html";
    }
    else {
        rc = false;
    }
    return rc;
}


}
