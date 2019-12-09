# Resources, tutorials

* web notifications: https://developers.google.com/web/fundamentals/push-notifications/display-a-notification

# libpurple

## libpurple signals

```
void (*wrote_im_msg)(PurpleAccount *account, const char *who,
                char *message, PurpleConversation *conv,
               PurpleMessageFlags flags);

void (*sent_im_msg)(PurpleAccount *account, const char *receiver,
              const char *message);
```
-----

received-im-msg: doesn't fill message flags?

-----

Q. What is the difference between wrote-im-msg and sent-im-msg?

- wrote-im-msg: called when a message is displayed in a chat window (no matter if it's an
incoming or outgoing message).


## multi-threading

Pidgin and libpurple are not thread safe.  It's better for you to hook
your asynchronous code into the glib/GTK+ main loop and use signals and
callbacks to start and step through your request and then retrieve data
when it's ready.  See eventloop.h:

https://developer.pidgin.im/doxygen/2.10.5/html/eventloop_8h.html


## buddies hierarchy

(to be confirmed)

* group
 * contact (?)
  * buddy


# libmicrohttpd

[libmicrohttpd] How to process a post request without MHD_post_processor?

The first call to the "test_send_response_helper" is made *before* the
upload data is available (after we just got the headers) so that you can
choose not to send 100 CONTINUE for http/1.1.  You need to change your
function to return an 'int' and return 'MHD_YES' to get the next call
which will contain upload data.  Furthermore, 'upload_data' is NOT a
0-terminated string so you *must* limit your processing manually to the
first '*upload_data_size' bytes in that buffer.  Finally, you then need
to reduce '*upload_data_size'  (i.e. to zero) to indicate how many bytes
of 'upload_data' you did process (so you don't get them again).
Finally, if you are eventually called  a *second* time with '0 ==
*upload_data_size', you can be sure that the upload is complete.


# use from cmd. line

Send a message:

`curl --data "Some message" http://localhost:8888/bibi/v/html/conversations/2`


# glib QRH

```
GList* g_list_first(GList*)
GList* g_list_next(GList*)
```