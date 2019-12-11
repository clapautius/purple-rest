# pidgin / finch / purple REST plugin

## Table of Contents  

[Description](#description)  
[Build](#build)  
[Install](#install)  
[Usage](#usage)  
[Dev. notes](#dev-notes)

## Description

This plugin provides a RESTful API for pidgin / finch.

It should also be able to provide various ways to remote control pidgin / finch. ATM there's an HTML/JS client that uses most of the API's features.

### HTML client

#### Screenshots

![purple rest html 1](doc/img/purple-rest-screenshot1.png?raw=true "HTML client - screenshot1")
![purple rest html 2](doc/img/purple-rest-screenshot2.png?raw=true "HTML client - screenshot2")

![purple rest html 3](doc/img/purple-rest-screenshot-3-frx-android.png?raw=true "HTML client - screenshot-3-frx-android")

## Build

### Prerequisites

* cmake
* c++ compiler (g++ / clang)
* development stuff (headers / libs) for
  * libpurple
  * glib
  * jsoncpp library
  * microhttpd library

### Build the plugin

```
mkdir purple-rest-build
cd purple-rest-build
cmake <PURPLE-REST-REPO>/purple-rest-src
make
```

## Install

### Installing the pidgin / finch plugin

Copy the file `libpurple-rest.so` from the build dir. to `~/.purple/plugins`.

#### Configuration

* _/plugins/purple-rest/server-port_ : listen port
* _/plugins/purple-rest/url-prefix_ : URL prefix (stripped from requests) - useful for
nginx / apache redirecting.

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

### Installing the web client

The web client's files (*index.html*, *purple-rest.js*, etc.) should be in `/var/www/html/purple-rest` (considering the default *apache* configuration).

Assuming the HTTP URL is ```http://<WEBSERVER>/purple-rest/``` :

#### Configuring apache web server

* enable proxy module (on debian symlink the files *proxy.load*, *proxy.conf* and *proxy_http.load*)
* add these to *default.conf* or *default-ssl.conf*:

```
<Location "/purple-rest/rest">
  ProxyPass http://localhost:8888
  ProxyPassReverse http://localhost:8888
</Location>
```

## Usage

### API

#### Definitions:

*my messages* : IM messages and messages from chat conversations that contain user's
nick.

*FORMAT* : html | json

#### GET requests

* `.../v1/FORMAT/messages` - all messages received since the plugin has started
* `.../v1/FORMAT/messages/all/start_from/MSG_ID` - messages starting with *MSG_ID*

* `.../v1/FORMAT/conv-messages/ID` - messages for the specified conversation
* `.../v1/FORMAT/conv-messages/ID/start_from/MSG_ID` - messages for the
specified conversation starting with *MSG_ID*

* `.../v1/FORMAT/conversations/all` - conversations list

* `.../v1/FORMAT/accounts/all` - accounts list

* `.../v1/FORMAT/my-messages/` - all *my messages*
* `.../v1/FORMAT/my-messages/start_from/MSG_ID` - *my messages* starting with *MSG_ID*

* `.../v1/FORMAT/buddies/all` - list of all buddies
* `.../v1/FORMAT/buddies/online` - list of online buddies
 * response for *json* format
 ```
[
  { "group" : "Buddies",
    "name" : "purple-rest3@localhost",
    "status" : "offline"
  },
  ...
]
```

* `.../v1/json/status/max_msg_id` - latest message id (json only)
* `.../v1/json/status/max_my_msg_id` - latest *my message* id received (json only)


* `.../v1/FORMAT/status/account-status` - status of the first account; it can be
  "Offline", "Unknown" or another status reported by libpurple.

* `.../v1/FORMAT/cmd/clear_history` - clear history

#### POST requests

* `.../v1/FORMAT/conv-messages/ID` - send a message for conversation *ID*

#### PUT requests

* `.../v1/FORMAT/conversations/BUDDY_NAME` - create a new conversation with *BUDDY_NAME*
 * for *json* format the server returns details of the newly created conversation
 ```
[
  {
    "id" : 2,
    "name" : "purple-rest@localhost"
  }
]
```

#### DELETE requests

* `.../v1/FORMAT/conversations/ID` - delete the conversation *ID*

## Dev. notes<a name="dev-notes"/>

See [dev-notes.md](doc/dev-notes.md)

