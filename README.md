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

Definitions:

**my messages** means IM messages and messages from chat conversations that contain user's
nick.

#### Requests

* `.../v1/FORMAT/messages` - all messages received since the plugin has started
* `.../v1/FORMAT/messages/all/start_from/MSG_ID` - messages starting with *MSG_ID*

* `.../v1/FORMAT/conversations/all` - conversations list
* `.../v1/FORMAT/conversations/ID` - messages for the specified conversation

* `.../v1/FORMAT/my-messages/` - all *my messages*
* `.../v1/FORMAT/my-messages/start_from/MSG_ID` - *my messages* starting with *MSG_ID*

FORMAT = html | json

### Configuration

* _/plugins/purple-rest/server-port_ : listen port
* _/plugins/purple-rest/url-prefix_ : URL prefix (stripped from requests) - useful for
nginx / apache redirecting.
