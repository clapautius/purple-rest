# Synopsis

## Definitions

**IM conversation, chat conversation** : IM conversation = conversation between two
people; chat = conversation with multiple participants.

## UML diagrams

UML activity dia. (basic ver.)

![purple rest uml 1](uml-purple-rest-activity-dia.png?raw=true "UML activity dia. (basic ver.)")

# Resources, tutorials

* Pidgin/Finch/libpurple API Documentation : https://docs.pidgin.im/pidgin/2.x.y/

* Pidgin/Finch/libpurple sources: https://bitbucket.org/pidgin/main/src/default/

* Web notifications: https://developers.google.com/web/fundamentals/push-notifications/display-a-notification

## Shortcuts

PurpleMessageFlags (`enum PurpleMessageFlags`) :
https://docs.pidgin.im/pidgin/2.x.y/conversation_8h.html#a66e44dfdc0de2953d73f03fb806bf6f5

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

## message flags

### relevant log fragments

```
(21:06:14) core-purple_rest: New IM msg in conversation: wrote-im-msg: (account, sender, buffer, conv, flags, data)0x5584945f8290,t@jabber.x,t@jabber.x has gone away.,0x5584948cde60,16404,0
(21:06:14) core-purple_rest: Got an IM msg: t@jabber.x has gone away.
```
*16404 = 0b100000000010100*

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


## status, presence

###  PurpleStatusType

`typedef struct _PurpleStatusType PurpleStatusType`

PurpleStatusType's are created by each PRPL (protocol).

They outline the available statuses of the protocol. AIM, for example, supports an
available state with an optional available message, an away state with a mandatory
message, and an invisible state (which is technically "independent" of the other two, but
we'll get into that later). PurpleStatusTypes are very permanent. They are hardcoded in
each PRPL and will not change often. And because they are hardcoded, they do not need to
be saved to any XML file.

#### PurpleStatusType example for skypeweb

```
GList *
skypeweb_status_types(PurpleAccount *account)
{
	GList *types = NULL;
	PurpleStatusType *status;

	status = purple_status_type_new_full(PURPLE_STATUS_OFFLINE, NULL, NULL, FALSE, FALSE, FALSE);
	types = g_list_append(types, status);

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AVAILABLE, SKYPEWEB_STATUS_ONLINE, _("Online"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AWAY, SKYPEWEB_STATUS_AWAY, _("Away"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);
//	status = purple_status_type_new_with_attrs(PURPLE_STATUS_EXTENDED_AWAY, SKYPEWEB_STATUS_AWAY, _("Not Available"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
//	types = g_list_append(types, status);
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_UNAVAILABLE, SKYPEWEB_STATUS_BUSY, _("Do Not Disturb"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_INVISIBLE, SKYPEWEB_STATUS_HIDDEN, _("Invisible"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);
	status = purple_status_type_new_with_attrs(PURPLE_STATUS_OFFLINE, SKYPEWEB_STATUS_OFFLINE, _("Offline"), TRUE, TRUE, FALSE, "message", "Mood", purple_value_new(PURPLE_TYPE_STRING), NULL);
	types = g_list_append(types, status);

	return types;
}
```

### libpurple 3.0

**A PurpleStatus can be thought of as an "instance" of a PurpleStatusType.**

If you're familiar with object-oriented programming languages then this should be
immediately clear. Say, for example, that one of your AIM buddies has set himself as
"away." You have a PurpleBuddy node for this person in your buddy list. Purple wants to
mark this buddy as "away," so it creates a new PurpleStatus. The PurpleStatus has its
PurpleStatusType set to the "away" state for the oscar PRPL. The PurpleStatus also
contains the buddy's away message. PurpleStatuses are sometimes saved, depending on the
context. The current PurpleStatuses associated with each of your accounts are saved so
that the next time you start Purple, your accounts will be set to their last known
statuses. There is also a list of saved statuses that are written to the status.xml
file. Also, each PurpleStatus has a "saveable" boolean. If "saveable" is set to FALSE then
the status is NEVER saved. **All PurpleStatuses should be inside a PurplePresence.**

**A PurpleStatus is either "independent" or "exclusive." Independent statuses can be active
or inactive and they don't affect anything else. However, you can only have one exclusive
status per PurplePresence. If you activate one exclusive status, then the previous
exclusive status is automatically deactivated.**

**A PurplePresence is like a collection of PurpleStatuses (plus some other random info).**


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
