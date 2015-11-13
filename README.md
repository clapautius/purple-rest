# pidgin / finch / purple REST plugin

## Description

This plugin provides a RESTful API for pidgin / finch.

It should (also) be able to provide various ways to remote control pidgin / finch (work in
progress).

## Build

### Prerequisites

* cmake
* gcc / g++
* development stuff (headers / libs) for
 * libpurple
 * glib
 * jsoncpp library
 * microhttpd library

## Usage

### API

Definitions:

**my messages** : IM messages and messages from chat conversations that contain user's
nick.

**FORMAT** : html | json

#### GET requests

* `.../v1/FORMAT/messages` - all messages received since the plugin has started
* `.../v1/FORMAT/messages/all/start_from/MSG_ID` - messages starting with *MSG_ID*

* `.../v1/FORMAT/conversations/all` - conversations list
* `.../v1/FORMAT/conversations/ID` - messages for the specified conversation

* `.../v1/FORMAT/my-messages/` - all *my messages*
* `.../v1/FORMAT/my-messages/start_from/MSG_ID` - *my messages* starting with *MSG_ID*

* `.../v1/json/status/max_msg_id` - latest message id (json only)
* `.../v1/json/status/max_my_msg_id` - latest *my message* id received (json only)

* `.../v1/FORMAT/cmd/clear_history` - clear history

#### POST requests

* (work in progress)

### Configuration

* _/plugins/purple-rest/server-port_ : listen port
* _/plugins/purple-rest/url-prefix_ : URL prefix (stripped from requests) - useful for
nginx / apache redirecting.

## Installing and using the web client

Assuming the HTTP URL is ```http://<WEBSERVER>/purple-rest/``` :

### pidgin / finch configuration

Things to be added to ```~/.purple/pref.xml```

```xml
<pref version='1' name='/'>
  <pref name='plugins'>
    <pref name='core'>
      ...
      <pref name='purple-rest'>
        <pref name='server-port' type='int' value='8888'/>
        <pref name='url-prefix' type='string' value='purple-rest/'/>
      </pref>
      ...
```

### Configuring apache web server

* enable proxy module (on debian symlink the files *proxy.load*, *proxy.conf* and *proxy_http.load*)
* add these to *default.conf* or *default-ssl.conf*:

```
<Location "/purple-rest/rest">
  ProxyPass http://localhost:8888
  ProxyPassReverse http://localhost:8888
</Location>
```
