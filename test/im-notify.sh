#!/bin/bash

g_debug=0
g_max_id=0
g_remote_max_id=0

# $1 - base url
# $2 - start from
get_max_msg_id()
{
    max_msg_id_req_param="max_msg_id"
#    max_msg_id_req_param="max_my_msg_id" # only for 'my messages'
    url="$1/v/json/status/$max_msg_id_req_param"
    ((g_debug)) && echo ":debug: curl url: $url"
    ids=$(curl "$url" 2>/dev/null | egrep "^ *\"max_[a-z_]*msg_id\" : .*" | sed "s/^ *\"max_[a-z_]*msg_id\" : \([0-9]*\)/\1/")
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
        # get the max id again, after the kdialog window has been closed
        # to avoid having another window for the next iteration if multiple messages arrive
        get_max_msg_id "$1" "$g_max_id"
        g_max_id="$g_remote_max_id"
    fi
    sleep 10
done
