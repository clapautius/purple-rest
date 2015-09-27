#!/bin/bash

g_debug=0
g_max_id=0
g_remote_max_id=0

# $1 - base url
# $2 - start from
get_max_msg_id()
{
    url="$1/v/json/my-messages/im/start_from/$2"
    ids=$(curl "$url" 2>/dev/null | egrep "^ *\"id\" : .*" | sed "s/^ *\"id\" : \([0-9]*\),/\1/")
    ((g_debug)) && echo ":debug: ids: $ids"
    g_remote_max_id=$(echo "$ids" | sort -nr | head -n 1)
    [ -z "$g_remote_max_id" ] && g_remote_max_id="0"
}


if [ "$1" == "-d" ]; then
    g_debug=1
    shift
fi

[ -z "$1" ] && echo "No URL specified" && exit 1

get_max_msg_id "$1" 0
g_max_id="$g_remote_max_id"
while /bin/true; do
    get_max_msg_id "$1" "$g_max_id"
    ((g_debug)) && echo ":debug: remote max id is $g_remote_max_id"
    if [ "$g_remote_max_id" -gt "$g_max_id" ]; then
        g_max_id="$g_remote_max_id"
        ((g_debug)) && echo ":debug: new max => new message"
        kdialog --msgbox "New IM message"
    fi
    sleep 10
done
