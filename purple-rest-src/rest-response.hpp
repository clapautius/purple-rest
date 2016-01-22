/* Purple REST plugin -- Copyright (C) 2015, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

#ifndef REST_RESPONSE_HPP
#define REST_RESPONSE_HPP

#include <sstream>
#include <memory>

#include<json/writer.h>

#include "immessage.hpp"
#include "imbuddy.hpp"

namespace p_rest
{

/**
 * Class used for composing a REST response in various formats.
 * This is the generic version.
 */
class RestResponse
{
public:

    virtual ~RestResponse();

    virtual std::string get_text() = 0;

    virtual void add_message(std::shared_ptr<ImMessage> &msg) = 0;

    virtual void add_conversation(PurpleConversation *conv,
                                  unsigned conv_id = 0) = 0;

    virtual void add_generic_param(const std::string &param_name,
                                   unsigned value) = 0;

    virtual void add_generic_param(const std::string &param_name,
                                   const char* p_str) = 0;

    virtual void add_buddy(const Buddy &buddy) = 0;

};


class HtmlResponse : public RestResponse
{
public:

    HtmlResponse();

    virtual ~HtmlResponse();

    virtual std::string get_text();

    virtual void add_message(std::shared_ptr<ImMessage> &msg);

    virtual void add_conversation(PurpleConversation *conv, unsigned conv_id = 0);

    virtual void add_generic_param(const std::string &param_name, unsigned value);
    virtual void add_generic_param(const std::string &param_name, const char* p_str);

    virtual void add_buddy(const Buddy &buddy);

private:

    std::ostringstream m_ostr;
};


class JsonResponse : public RestResponse
{
public:

    JsonResponse();

    virtual ~JsonResponse();

    virtual std::string get_text();

    virtual void add_message(std::shared_ptr<ImMessage> &msg);

    virtual void add_conversation(PurpleConversation *conv,
                                  unsigned conv_id = 0);

    virtual void add_generic_param(const std::string &param_name, unsigned value);
    virtual void add_generic_param(const std::string &param_name, const char* p_str);

    virtual void add_buddy(const Buddy &buddy);

protected:

    template<typename T>
    void add_generic_param_generic_value(const std::string &param_name, const T &value)
    {
        Json::Value new_param(Json::objectValue);
        new_param[param_name.c_str()] = value;
        m_response.append(new_param);
    }

private:

    Json::Value m_response;

};


bool create_response(const std::string &type,
                     std::unique_ptr<RestResponse> &response,
                     std::string &content_type);

}

#endif
