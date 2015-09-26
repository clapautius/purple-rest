/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "rest-response.hpp"
#include "html-resources.hpp"

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
           << "(" << msg->get_short_date_string() << "):"
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
        // :fixme:
        m_msg_list.append(new_conv);
    }
}

}
