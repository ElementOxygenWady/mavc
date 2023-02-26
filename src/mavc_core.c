/**
 * @file
 * @brief Module avc core
 *
 * No more details!
 */

#include "private.h"
#include "mavc_json.h"
#include <e2str/e2str_module_message.h>
#include <pjapp/pjapp.h>

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */

#define LOG_TAG                 "CORE" //日志标签

#define UI_MODULE MTOOL_MODULE_AVC_NAME ".T"


struct mavc_t;

typedef struct mavc_t mavc_t;


struct mavc_t
{
    int m_current_call_id;
};


static mavc_t * mavc_get_instance(void)
{
    static mavc_t instance = {
        .m_current_call_id = -1,
    };
    return &instance;
}

static void mavc_on_incoming_call(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_json_exec_1(pjapp_call_t, call_info, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_INCOMING_CALL, 0, 0, content, strlen(content)));
}

static void mavc_on_call_outgoing(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_get_instance()->m_current_call_id = call_info->m_call_id;
    mavc_json_exec_1(pjapp_call_t, call_info, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_OUTGOING, 0, 0, content, strlen(content)));
}

static mt_status_t module_load(mtool_module *module)
{
    pjapp_config_t config;
    pjapp_config_init(&config);
    config.m_account_configs.m_n_accounts = 0;
    config.m_transport_configs.m_flags |= PJAPP_TP_UDP;
    config.m_cbs_configs.m_cbs.on_incoming_call = mavc_on_incoming_call;
    config.m_cbs_configs.m_cbs.on_call_outgoing = mavc_on_call_outgoing;
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
            char * uri = (char *) content;
            pjapp_make_call(uri, 3000, NULL);
            break;
        }
        case MSG_MAVC_ACCEPT_CALL: {
            pjapp_accept_call(mavc_get_instance()->m_current_call_id);
            break;
        }
        case MSG_MAVC_HANGUP_CALL: {
            pjapp_hangup_call(mavc_get_instance()->m_current_call_id);
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
