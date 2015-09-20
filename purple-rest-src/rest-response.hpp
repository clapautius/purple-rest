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

namespace purple
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
};


class HtmlResponse : public RestResponse
{
public:

    HtmlResponse();

    virtual ~HtmlResponse();

    virtual std::string get_text();

    virtual void add_message(std::shared_ptr<ImMessage> &msg);

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

private:

    Json::Value m_msg_list;

};

}

#endif
