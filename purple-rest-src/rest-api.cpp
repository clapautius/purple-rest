#include <string.h>
#include <string>

#include <json/json.h>

#include "rest-api.hpp"
#include "history.hpp"
#include "purple-interaction.hpp"

extern purple::History g_msg_history;


// :fixme: :todo:
int perform_rest_request(char *buf, int buf_len)
{
    purple_info("Got new request, returning history");
    char *p = g_msg_history.get_full_history_list();
    strncpy(buf, p, buf_len);
    buf[buf_len - 1] = 0;
    free(p);
    return 1;
}
