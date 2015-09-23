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
    m_ostr << HTML_HEAD << "<body>" << HTML_STYLE;
    m_ostr << "<div align=\"center\"><div class=\"content\">";
}


HtmlResponse::~HtmlResponse()
{
}


std::string HtmlResponse::get_text()
{
    m_ostr << "</div></div></body></html>";
    return m_ostr.str();
}


void HtmlResponse::add_message(std::shared_ptr<ImMessage> &msg)
{
    m_ostr << "<div class=\"message\"><span class=\"message-props\">"
           << "Message from " << msg->get_sender() << "(id: "
           << msg->get_id() << ", conv_id: " << msg->get_conv_id() << ")</span><br>\n"
           << "<span class=\"message-text\">" << msg->get_text() << "</span></div>\n";
}


void HtmlResponse::add_conversation(PurpleConversation *conv)
{
    if (conv) {
        const char *conv_name = purple_conversation_get_name(conv);
        m_ostr << "<div class=\"message\"><span class=\"message-props\">"
               << "Conversation: " << (conv_name ? conv_name : "unknown")
               << "</span>" << "</div>\n";
    }
}



void HtmlResponse::add_send_msg_form(unsigned conversation_id)
{
    m_ostr << "<br><form method=\"POST\">"
           << "<input type=\"text\" name=\"text\">"
           << "</form><br>";
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


void JsonResponse::add_conversation(PurpleConversation *conv)
{
    if (conv) {
        Json::Value new_conv(Json::objectValue);
        const char *conv_name = purple_conversation_get_name(conv);
        new_conv["name"] = (conv_name ? conv_name : "unknown");
        // :fixme:
        m_msg_list.append(new_conv);
    }
}

}
