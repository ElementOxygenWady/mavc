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


static void mavc_on_incoming_call(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_INCOMING_CALL, 0, 0, content, strlen(content)));
}

static void mavc_on_call_outgoing(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_OUTGOING, 0, 0, content, strlen(content)));
}

static void mavc_on_call_confirmed(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_CONFIRMED, 0, 0, content, strlen(content)));
}

static void mavc_on_call_disconnected(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_DISCONNECTED, 0, 0, content, strlen(content)));
}

static void mavc_on_call_cancelled(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_CANCELLED, 0, 0, content, strlen(content)));
}

static void mavc_on_call_rejected(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_CALL_REJECTED, 0, 0, content, strlen(content)));
}

static void mavc_on_audio_eof(const void * audio)
{
    const pjapp_audio_play_t * audio_play = (const pjapp_audio_play_t *) audio;
    mavc_audio_eof_t audio_eof_obj = { .id = audio_play->m_play_id };
    mavc_json_exec_1(mavc_audio_eof_t, &audio_eof_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_AUDIO_PLAY_FINISHED, 0, 0, content, strlen(content)));
}

static mt_status_t module_load(mtool_module *module)
{
    pjapp_config_t config;
    config.m_account_configs.m_n_accounts = 0;
    config.m_transport_configs.m_flags = PJAPP_TP_UDP;
    config.m_cbs_configs.m_cbs.on_incoming_call = mavc_on_incoming_call;
    config.m_cbs_configs.m_cbs.on_call_outgoing = mavc_on_call_outgoing;
    config.m_cbs_configs.m_cbs.on_call_confirmed = mavc_on_call_confirmed;
    config.m_cbs_configs.m_cbs.on_call_disconnected = mavc_on_call_disconnected;
    config.m_cbs_configs.m_cbs.on_call_cancelled = mavc_on_call_cancelled;
    config.m_cbs_configs.m_cbs.on_call_rejected = mavc_on_call_rejected;
    config.m_cbs_configs.m_cbs.on_audio_eof = mavc_on_audio_eof;
    pjapp_init(&config);
    return MT_SUCCESS;
}

static mt_status_t module_start(mtool_module *module)
{
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
            char url[136];
            if (strlen(call_data.user_name) > 0)
            {
                snprintf(url, sizeof(url), "sip:%s@%s", call_data.user_name, call_data.remote_host);
            } else
            {
                snprintf(url, sizeof(url), "sip:%s", call_data.remote_host);
            }
            pjapp_make_call_v2(url, NULL);
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
        case MSG_MAVC_START_PLAY_AUDIO: {
            mavc_audio_file_t audio_file;
            mavc_json_cast_1(mavc_audio_file_t, (char *) content, &audio_file);
            int aud_id = 0;
            pjapp_err err = pjapp_play_wav(audio_file.filename, !!audio_file.loop, &aud_id);
            mavc_audio_file_ack_t ack = {
                .status = PJAPP_ERR_SUCCESS == err ? 0 : -1,
                .id = aud_id,
            };
            mavc_json_exec_1(mavc_audio_file_ack_t, &ack, content,
                mtool_module_send_reliable_ack2(message, MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                    0, 0, content, strlen(content)));
            break;
        }
        case MSG_MAVC_STOP_PLAY_AUDIO: {
            mavc_audio_file_t audio_file;
            mavc_json_cast_1(mavc_audio_file_t, (char *) content, &audio_file);
            pjapp_stop_wav(audio_file.id);
            break;
        }
        case MSG_MAVC_SET_VOLUME: {
            mavc_audio_volume_t volume;
            mavc_json_cast_1(mavc_audio_volume_t, (char *) content, &volume);
            pjapp_set_volume(volume.id, volume.volume);
            break;
        }
        case MSG_MAVC_START_RECORD: {
            mavc_record_t record;
            mavc_json_cast_1(mavc_record_t, (char *) content, &record);
            pjapp_record_start(record.filename, record.sources);
            break;
        }
        case MSG_MAVC_STOP_RECORD: {
            pjapp_record_stop();
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
