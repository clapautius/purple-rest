#!/bin/bash

g_debug=0
#g_debug=1

# $1 - base url
# $2 - start from
get_max_msg_id()
{
    url="$1/v/json/messages/im/start_from/$2"
    max_id=$(curl "$url" 2>/dev/null | egrep "^ *\"id\" : .*" | sed "s/^ *\"id\" : \([0-9]*\),/\1/" | sort -nr | head -n 1)
    [ -z "$max_id" ] && max_id="0"
    echo $max_id
}


[ -z "$1" ] && echo "No URL specified" && exit 1

max_id=$(get_max_msg_id "$1" 0)
while /bin/true; do
    id=$(get_max_msg_id "$1" "$max_id")
    ((g_debug)) && echo ":debug: remote max id is $id"
    if [ "$id" -gt "$max_id" ]; then
        max_id="$id"
        ((g_debug)) && echo ":debug: new max => new message"
        kdialog --msgbox "New IM message"
    fi
    sleep 10
done
