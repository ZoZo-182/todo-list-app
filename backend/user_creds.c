#include "user_creds.h"
#include <microhttpd.h>
#include <string.h>


static enum MHD_Result post_iterator(void *cls, enum MHD_ValueKind kind, const char *key,
        const char *filename, const char *content_type,
        const char *transfer_encoding, const char *data, uint64_t off,
        size_t size)
{
    return MHD_YES;
}

static int register_user(void *cls, struct MHD_Connection *connection,
        const char *url, const char *method, const char *version,
        const char *upload_data, size_t *upload_data_size,
        void **con_cls)
{
    ConnInfo *user_info = *con_cls;

    if (user_info->pp == NULL)
    {
        user_info->pp = MHD_create_post_processor(connection, *upload_data_size, 
                &post_iterator, user_info);
        *con_cls = user_info->pp;
        return MHD_YES;
    }
    if (*upload_data_size)
    {
        MHD_post_process(user_info->pp, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }
    else 
    {
        MHD_destroy_post_processor(user_info->pp);
        //return MHD_queue_response(connection, code, response);
        return MHD_NO;
    }
}

int main(void) {
   // register_user();
    return 0;
}

