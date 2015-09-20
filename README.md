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

### Configuration

* _/plugins/purple-rest/server-port_ : listen port
* _/plugins/purple-rest/url-prefix_ : URL prefix (stripped from requests) - useful for
nginx / apache redirecting.

## TODO

* check json spirit library
