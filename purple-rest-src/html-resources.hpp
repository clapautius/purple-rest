/* Purple REST plugin -- Copyright (C) 2019-2020, Tudor M. Pristavu

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 3 of the License, or (at your option) any later
   version.
*/

/**
 * HTML-related elements (styles, headers, etc.)
 */

#ifndef HTML_RESOURCES_HPP
#define HTML_RESOURCES_HPP

#include <string>


#define HTML_STYLE "<style>\
div.message {\
  border-bottom: 1px solid #bbb;\
  margin-bottom: 1ex;\
}\
\
span.message-props {\
  font-style: italic;\
}\
\
body\
{\
  background-color: #e0e0e0;\
}\
\
div.content\
{\
  border-width: 1px;\
  border-color: black;\
  border-style: solid;\
  width: 50em;\
  background-color: #ffffff;\
  text-align: left;\
}\
\
</style>"


#define HTML_HEAD "<html><head>\n\
<title>purple-rest plugin</title>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" >\n\
</head>"


/**
 * Configurable elements for the HTML output (can be set up in purple config file).
 */
class HtmlTemplate
{
public:
    HtmlTemplate();

    void read_config();

    std::string get_message_system_prefix() const;

private:

    std::string m_message_system_prefix;
};


/**
 * @todo :fixme: get rid of this global
 */
extern HtmlTemplate g_html_template;

#endif
