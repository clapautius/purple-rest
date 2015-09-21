/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#include "rest-response.hpp"

namespace purple
{

RestResponse::~RestResponse()
{
}

#define HTML_STYLE "<style>\
div.message {\
  border-bottom: 1px solid #bbb;\
  margin-bottom: 1ex;\
}\
\
span.message-props {\
  font-style: italic;\
}\
</style>"


HtmlResponse::HtmlResponse()
{
    m_ostr << "<html><head>" << HTML_STYLE << "</head><body>";
}


HtmlResponse::~HtmlResponse()
{
}


std::string HtmlResponse::get_text()
{
    m_ostr << "</body></html>";
    return m_ostr.str();
}


void HtmlResponse::add_message(std::shared_ptr<ImMessage> &msg)
{
    m_ostr << "<div class=\"message\"><span class=\"message-props\">"
           << "Message from " << msg->get_sender() << "(id: "
           << msg->get_id() << ")</span><br>\n"
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
