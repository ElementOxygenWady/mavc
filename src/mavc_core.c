/**
 * @file
 * @brief Module avc core
 *
 * No more details!
 */

#define LOG_DEBUG
#define LOG_TAG                 "CORE" //日志标签

#include "private.h"
#include "mavc_json.h"
#include "mavc_comm_objects.h"
#include <e2str/e2str_module_message.h>
#include <pjapp/pjapp.h>

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */

#define UI_MODULE MTOOL_MODULE_GUI_NAME


struct mavc_t;

typedef struct mavc_t mavc_t;


// Internal call, assert all of the parameters is valid.
static int mavc_extract_info_from_url(const char * url,
    char * remote_user_name, unsigned remote_user_name_size,
    char * remote_host, unsigned remote_host_size)
{
    int ret = -1;
    unsigned offset = 0;
    const unsigned n_expect_extracts = 2;
    unsigned n_expect_meets = 0;
    do {
        unsigned url_len = strlen(url);
        ASSERT_BREAK(url_len > 5);
        ASSERT_BREAK(0 == strncmp(url, "sip:", 4));
        offset = 4;
        int do_continue = 1;
        unsigned i = offset;
        for (; i < url_len && 0 != do_continue; ++i)
        {
            switch (url[i])
            {
                case '@': {
                    if (0 != n_expect_meets ||  // Username does not in correct position.
                        i <= offset)  // Zero length of username.
                    {
                        do_continue = 0;
                        break;
                    }
                    unsigned user_name_len = i - offset + 1;  // +1 to include '\0'.
                    if (user_name_len > remote_user_name_size)
                    {
                        user_name_len = remote_user_name_size;
                    }
                    strncpy(remote_user_name, url + offset, user_name_len - 1);
                    remote_user_name[user_name_len - 1] = '\0';
                    offset = i + 1;
                    ++n_expect_meets;
                    break;
                }
                case ' ':
                case '\t':
                case '\r':
                case '\n': {
                    if (i > offset &&
                        1 == n_expect_meets)  // Host is in second position.
                    {
                        unsigned host_len = i - offset + 1;  // +1 to include '\0'.
                        if (host_len > remote_host_size)
                        {
                            host_len = remote_host_size;
                        }
                        strncpy(remote_host, url + offset, host_len - 1);
                        remote_host[host_len - 1] = '\0';
                        offset = i;
                        ++n_expect_meets;
                    }
                    do_continue = 0;
                    break;
                }
            }
        }
        ASSERT_BREAK(n_expect_meets >= n_expect_extracts ||
            (do_continue &&
                1 == n_expect_meets));  // Must meet and only meet username.
        if (n_expect_meets < n_expect_extracts)
        {
            if (i > (offset + 1))  // +1 to ignore '\0'.
            {
                unsigned host_len = i - offset + 1;  // +1 to include '\0'.
                if (host_len > remote_host_size)
                {
                    host_len = remote_host_size;
                }
                strncpy(remote_host, url + offset, host_len - 1);
                remote_host[host_len - 1] = '\0';
                offset = i;
                ++n_expect_meets;
            }
        }
        ASSERT_BREAK(n_expect_extracts == n_expect_meets);
        ret = 0;
    } while (0);
    return ret;
}

static void mavc_on_incoming_call(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj;
    if (0 == mavc_extract_info_from_url(call_info->m_remote_url,
        call_obj.user_name, sizeof(call_obj.user_name),
        call_obj.remote_host, sizeof(call_obj.remote_host)))
    {
        call_obj.id = call_info->m_call_id;
        mavc_json_exec_1(mavc_call_t, &call_obj, content,
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
                NULL, MSG_MAVC_INCOMING_CALL, 0, 0, content, strlen(content)));
    }
}

static void mavc_on_call_outgoing(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj;
    if (0 == mavc_extract_info_from_url(call_info->m_remote_url,
        call_obj.user_name, sizeof(call_obj.user_name),
        call_obj.remote_host, sizeof(call_obj.remote_host)))
    {
        call_obj.id = call_info->m_call_id;
        mavc_json_exec_1(mavc_call_t, &call_obj, content,
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
                NULL, MSG_MAVC_CALL_OUTGOING, 0, 0, content, strlen(content)));
    }
}

static void mavc_on_call_confirmed(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj;
    if (0 == mavc_extract_info_from_url(call_info->m_remote_url,
        call_obj.user_name, sizeof(call_obj.user_name),
        call_obj.remote_host, sizeof(call_obj.remote_host)))
    {
        call_obj.id = call_info->m_call_id;
        mavc_json_exec_1(mavc_call_t, &call_obj, content,
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
                NULL, MSG_MAVC_CALL_CONFIRMED, 0, 0, content, strlen(content)));
    }
}

static mt_status_t module_load(mtool_module *module)
{
    pjapp_config_t config;
    pjapp_config_init(&config);
    config.m_account_configs.m_n_accounts = 0;
    config.m_transport_configs.m_flags |= PJAPP_TP_UDP;
    config.m_cbs_configs.m_cbs.on_incoming_call = mavc_on_incoming_call;
    config.m_cbs_configs.m_cbs.on_call_outgoing = mavc_on_call_outgoing;
    config.m_cbs_configs.m_cbs.on_call_confirmed = mavc_on_call_confirmed;
    pjapp_init(&config);
    return MT_SUCCESS;
}

static mt_status_t module_start(mtool_module *module)
{
    pjapp_start();
    return MT_SUCCESS;
}

static mt_status_t module_stop(mtool_module *module)
{
    return MT_SUCCESS;
}

static mt_status_t module_unload(mtool_module *module)
{
    return MT_SUCCESS;
}

static mt_status_t module_on_rx_msg(mtool_module *module, mtool_module_message *message, void *content)
{
    mt_status_t status = MT_SUCCESS;

    switch (message->name) {
        case MSG_HUB_XXX_HEARTBEAT:
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                    NULL, message->modid_dst, NULL, message->modid_src,
                    NULL, MSG_XXX_HUB_HEARTBEAT_ACK, 0, 0, NULL, 0);
        break;
        case MSG_MAVC_MAKE_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            char url[64];
            snprintf(url, sizeof(url), "sip:%s@%s",
                strlen(call_data.user_name) > 0 ? call_data.user_name : "MAVC",
                call_data.remote_host);
            pjapp_make_call_v2(url, 3000, NULL);
            break;
        }
        case MSG_MAVC_ACCEPT_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            pjapp_accept_call(call_data.id);
            break;
        }
        case MSG_MAVC_HANGUP_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            pjapp_hangup_call(call_data.id);
            break;
        }
        case MSG_MAVC_REJECT_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            pjapp_reject_call(call_data.id);
            break;
        }
        case MSG_MAVC_CANCEL_MAKE_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            pjapp_cancel_call(call_data.id);
            break;
        }
        default:
            status = MT_EUNKNOWN;
        break;
    }

    return status;
}

static mtool_module module_instance = {
    .attr = {
        .name = {MTOOL_MODULE_AVC_NAME},
        .mode = MTOOL_MODULE_ATTACH_THREAD | MTOOL_MODULE_ENABLE_RELIABLE_SENDER,
        .transport = {
            .type = MTOOL_MODULE_TRANSPORT_TYPE_UDP,
            .udp = {
                .port = MTOOL_MODULE_AVC_UDP_PORT,
                .addr = {MTOOL_MODULE_AVC_UDP_ADDRESS},
            },
        },
        .modid = -1,
    },
    .load       = &module_load,
    .start      = &module_start,
    .stop       = &module_stop,
    .unload     = &module_unload,
    .on_rx_msg  = &module_on_rx_msg,
    .pd = NULL,
};

mtool_module *mavc_core_get_instance(void)
{
    return &module_instance;
}

mt_status_t mavc_core_register(void)
{
    mtool_module_manager_config mtmgr_config;
    mtool_module_transport server_module = {
        .type = MTOOL_MODULE_TRANSPORT_TYPE_UDP,
        .udp = {
            .port = MTOOL_MODULE_SERVER_UDP_PORT,
            .addr = {MTOOL_MODULE_SERVER_UDP_ADDRESS},
        },
    };

    mtool_module_manager_config_default(&mtmgr_config);

    memcpy(&mtmgr_config.transport, &module_instance.attr.transport, sizeof(mtmgr_config.transport));

    mtool_module_manager_init(&mtmgr_config);

    mtool_module_helper_add_server_module(&server_module);

    return mtool_module_register(&module_instance);
}

void mavc_core_unregister(void)
{
    mtool_module_unregister(&module_instance);
}

/**
 * @}
 */
