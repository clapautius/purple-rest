# pidgin / finch / purple REST plugin

## Description

This plugin provides a RESTful API for pidgin / finch.

It should (also) be able to provide various ways to remote control pidgin / finch (work in
progress).

## Build

### Prerequisites

* cmake
* gcc / g++
* libpurple (incl. development headers / libs)
* glib development stuff
* jsoncpp library
* microhttpd library

## Usage

### API

*my messages* means IM messages and messages from chat conversations that contain user's
nick.

* .../v1/<format>/messages - all messages received since the plugin has started
* .../v1/<format>/messages/all/start_from/<msg_id> - messages starting with <msg_id>

* .../v1/<format>/conversations/all - conversations list
* .../v1/<format>/conversations/<id> - messages for the specified conversation

 * .../v1/<format>/my-messages/ - all *my messages*
 * .../v1/<format>/my-messages/start_from/<msg_id> - *my messages* starting from <msg_id>

### Configuration

* _/plugins/purple-rest/server-port_ : listen port
* _/plugins/purple-rest/url-prefix_ : URL prefix (stripped from requests) - useful for
nginx / apache redirecting.
