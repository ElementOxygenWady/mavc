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
#include <mavc/mavc_pub.h>
#include <e2str/e2str_module_message.h>
#include <pjapp/pjapp.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */

#define UI_MODULE MTOOL_MODULE_GUI_NAME

#define MAVC_CORE_MAX_ACCOUNTS 4


enum mavc_internal_message_e;
struct mavc_t;

typedef enum mavc_internal_message_e mavc_internal_message_e;
typedef struct mavc_t mavc_t;


enum mavc_internal_message_e
{
    MAVC_INTERNAL_MSG_BEGIN = MSG_MAVC_INTERNAL,
    MAVC_INTERNAL_MSG_FETCH_ACCOUNTS,
    MAVC_INTERNAL_MSG_REGISTER_ACCOUNTS,
    MAVC_INTERNAL_MSG_TRY_RECYCLE_FETCH_CONFIG_THREAD,
};

struct mavc_t
{
    const mavc_config_t * m_config;
    pthread_t m_tid_fetch_config;
    struct {
        mavc_server_account_t m_accounts[MAVC_CORE_MAX_ACCOUNTS];
        unsigned m_n_accounts;
        bool m_run;
        bool m_do_fetch_acc;
        bool m_in_process;
    } m_thread_data;
    pthread_mutex_t m_locker;
};


static mavc_t * mavc_get(void)
{
    static mavc_t instance = {
        .m_config = NULL,
        .m_tid_fetch_config = 0,
        .m_thread_data = {
            .m_n_accounts = 0,
            .m_run = false,
            .m_do_fetch_acc = false,
            .m_in_process = false,
        }
    };
    return &instance;
}

static void * mavc_thread_fetch_config(void * arg)
{
    mavc_t * instance = mavc_get();
    pthread_mutex_lock(&instance->m_locker);
    instance->m_thread_data.m_in_process = true;
    pthread_mutex_unlock(&instance->m_locker);
    while (instance->m_thread_data.m_run)
    {
        bool do_break = false;
        pthread_mutex_lock(&instance->m_locker);
        if (!instance->m_thread_data.m_do_fetch_acc)
        {
            do_break = true;
        } else
        {
            instance->m_thread_data.m_run = false;
            instance->m_thread_data.m_do_fetch_acc = false;
        }
        pthread_mutex_unlock(&instance->m_locker);
        ASSERT_BREAK(!do_break);

        mtool_module_message_holder * holder = NULL;
        if (MT_SUCCESS != mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, MTOOL_MODULE_GWE_NAME, -1,
                NULL, MSG_GWE_GET_SIP_ACCOUNTS, 0, 0, NULL, 0, 2000, &holder))
        {
            usleep(2000000);
            pthread_mutex_lock(&instance->m_locker);
            instance->m_thread_data.m_do_fetch_acc = true;
            pthread_mutex_unlock(&instance->m_locker);
            continue;
        }
        if (NULL != holder)
        {
            cJSON * json = cJSON_Parse((char *) holder->content);
            do {
                instance->m_thread_data.m_n_accounts = 0;
                cJSON * arr = cJSON_GetObjectItem(json, "accounts");
                ASSERT_BREAK(NULL != arr);
                int n_elems = cJSON_GetArraySize(arr);
                for (int i = 0; i < n_elems && i < MAVC_CORE_MAX_ACCOUNTS; ++i)
                {
                    cJSON * elem = cJSON_GetArrayItem(arr, i);
                    memset(&instance->m_thread_data.m_accounts[i], 0, sizeof(instance->m_thread_data.m_accounts[i]));
                    cJSON * obj_username = cJSON_GetObjectItem(elem, "username");
                    if (NULL != obj_username)
                    {
                        snprintf(instance->m_thread_data.m_accounts[i].m_acc_info.m_username,
                            sizeof(instance->m_thread_data.m_accounts[i].m_acc_info.m_username),
                            "%s", cJSON_GetStringValue(obj_username));
                    }
                    cJSON * obj_password = cJSON_GetObjectItem(elem, "password");
                    if (NULL != obj_password)
                    {
                        snprintf(instance->m_thread_data.m_accounts[i].m_acc_info.m_password,
                            sizeof(instance->m_thread_data.m_accounts[i].m_acc_info.m_password),
                            "%s", cJSON_GetStringValue(obj_password));
                    }
                    cJSON * obj_server = cJSON_GetObjectItem(elem, "server");
                    if (NULL != obj_server)
                    {
                        snprintf(instance->m_thread_data.m_accounts[i].m_acc_info.m_server_host,
                            sizeof(instance->m_thread_data.m_accounts[i].m_acc_info.m_server_host),
                            "%s", cJSON_GetStringValue(obj_server));
                    } else
                    {
                        continue;
                    }
                    cJSON * obj_port = cJSON_GetObjectItem(elem, "port");
                    if (NULL != obj_port)
                    {
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_port = cJSON_GetNumberValue(obj_port);
                    } else
                    {
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_port = 5060;
                    }
                    cJSON * obj_tp = cJSON_GetObjectItem(elem, "transport");
                    if (NULL != obj_tp)
                    {
                        switch ((int) cJSON_GetNumberValue(obj_tp))
                        {
                            case MAVC_TP_UDP:
                                instance->m_thread_data.m_accounts[i].m_tp = MAVC_TP_UDP;
                                break;
                            case MAVC_TP_TCP:
                                instance->m_thread_data.m_accounts[i].m_tp = MAVC_TP_TCP;
                                break;
                            case MAVC_TP_TLS:
                                instance->m_thread_data.m_accounts[i].m_tp = MAVC_TP_TLS;
                                break;
                            case MAVC_TP_AUTO:
                            default:
                                instance->m_thread_data.m_accounts[i].m_tp = MAVC_TP_AUTO;
                                break;
                        }
                    } else
                    {
                        instance->m_thread_data.m_accounts[i].m_tp = MAVC_TP_AUTO;
                    }
                    cJSON * obj_active = cJSON_GetObjectItem(elem, "active");
                    if (NULL != obj_active)
                    {
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_active = cJSON_IsTrue(obj_active);
                    }
                    cJSON * obj_is_default = cJSON_GetObjectItem(elem, "is_default");
                    if (NULL != obj_is_default)
                    {
                        instance->m_thread_data.m_accounts[i].m_is_default = cJSON_IsTrue(obj_is_default);
                    }
                    ++instance->m_thread_data.m_n_accounts;
                }
            } while (0);
            cJSON_Delete(json);
            mtool_module_message_holder_destroy(holder);
            holder = NULL;

            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MAVC_INTERNAL_MSG_REGISTER_ACCOUNTS, 0, 0, NULL, 0);
        }

        do_break = true;
        pthread_mutex_lock(&instance->m_locker);
        if (instance->m_thread_data.m_do_fetch_acc)
        {
            do_break = false;
        } else
        {
            instance->m_thread_data.m_run = false;
        }
        pthread_mutex_unlock(&instance->m_locker);
        ASSERT_BREAK(!do_break);
    }

    pthread_mutex_lock(&instance->m_locker);
    instance->m_thread_data.m_in_process = false;
    pthread_mutex_unlock(&instance->m_locker);

    mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
        MTOOL_MODULE_AVC_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
        NULL, MAVC_INTERNAL_MSG_TRY_RECYCLE_FETCH_CONFIG_THREAD, 0, 0, NULL, 0);

    pthread_exit(NULL);
}

static void mavc_on_incoming_call(const void * call)
{
    const pjapp_call_t * call_info = (const pjapp_call_t *) call;
    mavc_call_t call_obj = { .id = call_info->m_call_id };
    snprintf(call_obj.user_name, sizeof(call_obj.user_name), "%s", call_info->m_remote_username);
    snprintf(call_obj.remote_host, sizeof(call_obj.remote_host), "%s", call_info->m_remote_host);
    mavc_json_exec_1(mavc_call_t, &call_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_INCOMING_CALL, 0, 0, content, strlen(content) + 1));
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
            NULL, MSG_MAVC_CALL_OUTGOING, 0, 0, content, strlen(content) + 1));
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
            NULL, MSG_MAVC_CALL_CONFIRMED, 0, 0, content, strlen(content) + 1));
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
            NULL, MSG_MAVC_CALL_DISCONNECTED, 0, 0, content, strlen(content) + 1));
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
            NULL, MSG_MAVC_CALL_CANCELLED, 0, 0, content, strlen(content) + 1));
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
            NULL, MSG_MAVC_CALL_REJECTED, 0, 0, content, strlen(content) + 1));
}

static void mavc_on_audio_eof(const void * audio)
{
    const pjapp_audio_play_t * audio_play = (const pjapp_audio_play_t *) audio;
    mavc_audio_eof_t audio_eof_obj = { .id = audio_play->m_play_id };
    mavc_json_exec_1(mavc_audio_eof_t, &audio_eof_obj, content,
        mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
            MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
            NULL, MSG_MAVC_AUDIO_PLAY_FINISHED, 0, 0, content, strlen(content) + 1));
}

static void mavc_on_register_status(int acc_id, pjapp_reg_status_e status)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "account_id", acc_id);
    cJSON_AddNumberToObject(obj, "status", status);
    char * content = cJSON_PrintUnformatted(obj);
    mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
        MTOOL_MODULE_AVC_NAME, -1, UI_MODULE, -1,
        NULL, MSG_MAVC_NOTIFY_REGISTER_STATUS, 0, 0, content, strlen(content) + 1);
    free(content);
    cJSON_free(obj);
}

static mt_status_t module_load(mtool_module *module)
{
    mavc_t * instance = mavc_get();
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
    config.m_cbs_configs.m_cbs.on_register_status = mavc_on_register_status;
    config.m_media_configs.m_ec_flags = PJAPP_EC_FLAG_WEBRTC |
                                        PJAPP_EC_FLAG_AGGRESSIVENESS_AGGRESSIVE |
                                        PJAPP_EC_FLAG_USE_NOISE_SUPPRESSOR;
    {
        if (NULL != instance->m_config)
        {
            do {
                ASSERT_BREAK(instance->m_config->m_server.m_active);
                ASSERT_BREAK(NULL != instance->m_config->m_server.m_username);
                ASSERT_BREAK(NULL != instance->m_config->m_server.m_server_host);
                char acc_url[128];
                char server_url[128];
                snprintf(acc_url, sizeof(acc_url), "sip:%s@%s",
                    instance->m_config->m_server.m_username,
                    instance->m_config->m_server.m_server_host);
                if (instance->m_config->m_server.m_port > 0)
                {
                    snprintf(server_url, sizeof(server_url), "sip:%s:%d",
                        instance->m_config->m_server.m_server_host,
                        instance->m_config->m_server.m_port);
                } else
                {
                    snprintf(server_url, sizeof(server_url), "sip:%s",
                        instance->m_config->m_server.m_server_host);
                }
                pjapp_config_add_account(&config, acc_url, server_url, NULL,
                    instance->m_config->m_server.m_username, instance->m_config->m_server.m_password);
            } while (0);
            instance->m_config = NULL;  // Do not keep config in memory.
        }
    }
    pjapp_init(&config);
    return MT_SUCCESS;
}

static mt_status_t module_start(mtool_module *module)
{
    mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
        MTOOL_MODULE_AVC_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
        NULL, MAVC_INTERNAL_MSG_FETCH_ACCOUNTS, 0, 0, NULL, 0);
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
            pjapp_make_call_v2(url, call_data.has_audio, call_data.has_video, NULL);
            break;
        }
        case MSG_MAVC_ACCEPT_CALL: {
            mavc_call_t call_data;
            mavc_json_cast_1(mavc_call_t, (char *) content, &call_data);
            pjapp_accept_call(call_data.id, call_data.has_audio, call_data.has_video);
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
                    0, 0, content, strlen(content) + 1));
            break;
        }
        case MSG_MAVC_PAUSE_PLAY_AUDIO: {
            mavc_audio_file_t audio_file;
            mavc_json_cast_1(mavc_audio_file_t, (char *) content, &audio_file);
            pjapp_pause_wav(audio_file.id);
            break;
        }
        case MSG_MAVC_RESUME_PLAY_AUDIO: {
            mavc_audio_file_t audio_file;
            mavc_json_cast_1(mavc_audio_file_t, (char *) content, &audio_file);
            pjapp_resume_wav(audio_file.id);
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
        case MSG_MAVC_REGISTER_ACCOUNT: {
            mavc_server_account_t server_acc_info;
            mavc_json_cast_1(mavc_server_account_t, (char *) content, &server_acc_info);
            pjapp_server_account_t server_acc;
            snprintf(server_acc.m_acc_url, sizeof(server_acc.m_acc_url), "sip:%s@%s",
                server_acc_info.m_acc_info.m_username, server_acc_info.m_acc_info.m_server_host);
            if (server_acc_info.m_acc_info.m_port > 0)
            {
                snprintf(server_acc.m_server_url, sizeof(server_acc.m_server_url), "sip:%s:%d",
                    server_acc_info.m_acc_info.m_server_host,
                    server_acc_info.m_acc_info.m_port);
            } else
            {
                snprintf(server_acc.m_server_url, sizeof(server_acc.m_server_url), "sip:%s",
                    server_acc_info.m_acc_info.m_server_host);
            }
            pjapp_transport_enum tp;
            pjapp_transport_enum * tp_ = &tp;
            switch (server_acc_info.m_tp)
            {
                case MAVC_TP_UDP:
                    tp = PJAPP_TP_UDP;
                    break;
                case MAVC_TP_TCP:
                    tp = PJAPP_TP_TCP;
                    break;
                case MAVC_TP_TLS:
                    tp = PJAPP_TP_TLS;
                    break;
                default:
                    tp_ = NULL;
            }
            snprintf(server_acc.m_realm, sizeof(server_acc.m_realm), "*");
            snprintf(server_acc.m_username, sizeof(server_acc.m_username), "%s", server_acc_info.m_acc_info.m_username);
            snprintf(server_acc.m_password, sizeof(server_acc.m_password), "%s", server_acc_info.m_acc_info.m_password);
            int acc_id = PJAPP_INVALID_ID;
            pjapp_err err = pjapp_register_account(&server_acc, server_acc_info.m_is_default, tp_, &acc_id);

            cJSON * obj = cJSON_CreateObject();
            cJSON_AddBoolToObject(obj, "status", PJAPP_ERR_SUCCESS == err);
            if (PJAPP_ERR_SUCCESS == err)
            {
                cJSON_AddNumberToObject(obj, "account_id", acc_id);
            }
            char * ack = cJSON_PrintUnformatted(obj);
            mtool_module_send_reliable_ack2(message, MTOOL_MODULE_MESSAGE_JSON_CONTENT, 0, 0, ack, strlen(ack) + 1);
            cJSON_Delete(obj);
            break;
        }
        case MSG_MAVC_UNREGISTER_ACCOUNT: {
            int acc_id = mavc_json_simple_get(content, "account_id", PJAPP_INVALID_ID, int);
            cJSON * obj = cJSON_CreateObject();
            if (PJAPP_INVALID_ID != acc_id)
            {
                pjapp_err err = pjapp_unregister_account(acc_id);
                cJSON_AddBoolToObject(obj, "status", PJAPP_ERR_SUCCESS == err);
            } else
            {
                cJSON_AddBoolToObject(obj, "status", false);
            }
            char * ack = cJSON_PrintUnformatted(obj);
            mtool_module_send_reliable_ack2(message, MTOOL_MODULE_MESSAGE_JSON_CONTENT, 0, 0, ack, strlen(ack) + 1);
            cJSON_Delete(obj);
            break;
        }
        case MSG_MAVC_GET_ACCOUNT_LIST: {
            pjapp_account_t accounts[8];
            unsigned n_accounts = sizeof(accounts) / sizeof(accounts[0]);
            cJSON * obj = cJSON_CreateObject();
            pjapp_err err = pjapp_get_account_list(accounts, &n_accounts);
            cJSON_AddBoolToObject(obj, "status", PJAPP_ERR_SUCCESS == err);
            if (PJAPP_ERR_SUCCESS == err)
            {
                cJSON * arr = cJSON_AddArrayToObject(obj, "accounts");
                for (unsigned i = 0; i < n_accounts; ++i)
                {
                    cJSON * acc_obj = cJSON_CreateObject();
                    cJSON_AddBoolToObject(acc_obj, "is_default", accounts[i].m_is_default_account);
                    cJSON_AddNumberToObject(acc_obj, "account_id", accounts[i].m_acc_id);
                    if (accounts[i].m_is_local_account)
                    {
                        cJSON_AddStringToObject(acc_obj, "acc_url", accounts[i].m_details.m_local_account.m_acc_url);
                        mavc_transport_e tp = MAVC_TP_AUTO;
                        switch (accounts[i].m_details.m_local_account.m_transport)
                        {
                            case PJAPP_TP_TCP:
                                tp = MAVC_TP_TCP;
                                break;
                            case PJAPP_TP_TLS:
                                tp = MAVC_TP_TLS;
                                break;
                            case PJAPP_TP_UDP:
                                tp = MAVC_TP_UDP;
                                break;
                            default:
                                break;
                        }
                        cJSON_AddNumberToObject(acc_obj, "transport", tp);
                    } else
                    {
                        cJSON_AddStringToObject(acc_obj, "acc_url", accounts[i].m_details.m_server_account.m_acc_url);
                        cJSON_AddStringToObject(acc_obj, "server_url",
                            accounts[i].m_details.m_server_account.m_server_url);
                        cJSON_AddStringToObject(acc_obj, "realm", accounts[i].m_details.m_server_account.m_realm);
                        cJSON_AddStringToObject(acc_obj, "username", accounts[i].m_details.m_server_account.m_username);
                        cJSON_AddNumberToObject(acc_obj, "account_status",
                            accounts[i].m_details.m_server_account.m_status);
                        mavc_transport_e tp = MAVC_TP_AUTO;
                        switch (accounts[i].m_details.m_server_account.m_transport)
                        {
                            case PJAPP_TP_TCP:
                                tp = MAVC_TP_TCP;
                                break;
                            case PJAPP_TP_TLS:
                                tp = MAVC_TP_TLS;
                                break;
                            case PJAPP_TP_UDP:
                                tp = MAVC_TP_UDP;
                                break;
                            default:
                                break;
                        }
                        cJSON_AddNumberToObject(acc_obj, "transport", tp);
                    }
                    cJSON_AddItemToArray(arr, acc_obj);
                }
            }
            char * ack = cJSON_PrintUnformatted(obj);
            mtool_module_send_reliable_ack2(message, MTOOL_MODULE_MESSAGE_JSON_CONTENT, 0, 0, ack, strlen(ack) + 1);
            cJSON_Delete(obj);
            break;
        }
        case MSG_MAVC_SET_DEFAULT_ACCOUNT: {
            int acc_id = mavc_json_simple_get(content, "account_id", PJAPP_INVALID_ID, int);
            cJSON * obj = cJSON_CreateObject();
            if (PJAPP_INVALID_ID != acc_id)
            {
                pjapp_err err = pjapp_set_default_account(acc_id);
                cJSON_AddBoolToObject(obj, "status", PJAPP_ERR_SUCCESS == err);
            } else
            {
                cJSON_AddBoolToObject(obj, "status", false);
            }
            char * ack = cJSON_PrintUnformatted(obj);
            mtool_module_send_reliable_ack2(message, MTOOL_MODULE_MESSAGE_JSON_CONTENT, 0, 0, ack, strlen(ack) + 1);
            cJSON_Delete(obj);
            break;
        }
        case MSG_GWE_NOTIFY_IP_CHANGED: {
            pjapp_err err = pjapp_ip_change();
            if (PJAPP_ERR_SUCCESS != err)
            {
                MAVC_LOGE(LOG_TAG, "Failed to handle IP changed, reason: %s.", pjapp_error(err));
            }
            break;
        }
        case MSG_GWE_AVAILABLE: {
            // Load configurations
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                MTOOL_MODULE_AVC_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MAVC_INTERNAL_MSG_FETCH_ACCOUNTS, 0, 0, NULL, 0);
            break;
        }
        case MAVC_INTERNAL_MSG_FETCH_ACCOUNTS: {
            mavc_t * instance = mavc_get();
            pthread_mutex_lock(&instance->m_locker);
            instance->m_thread_data.m_run = true;
            instance->m_thread_data.m_do_fetch_acc = true;
            pthread_mutex_unlock(&instance->m_locker);
            if (instance->m_tid_fetch_config <= 0)
            {
                pthread_create(&instance->m_tid_fetch_config, NULL, mavc_thread_fetch_config, NULL);
            }
            break;
        }
        case MAVC_INTERNAL_MSG_REGISTER_ACCOUNTS: {
            mavc_t * instance = mavc_get();
            for (unsigned i = 0; i < instance->m_thread_data.m_n_accounts; ++i)
            {
                pjapp_server_account_t server_acc;
                snprintf(server_acc.m_acc_url, sizeof(server_acc.m_acc_url), "sip:%s@%s",
                    instance->m_thread_data.m_accounts[i].m_acc_info.m_username,
                    instance->m_thread_data.m_accounts[i].m_acc_info.m_server_host);
                if (instance->m_thread_data.m_accounts[i].m_acc_info.m_port > 0)
                {
                    snprintf(server_acc.m_server_url, sizeof(server_acc.m_server_url), "sip:%s:%d",
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_server_host,
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_port);
                } else
                {
                    snprintf(server_acc.m_server_url, sizeof(server_acc.m_server_url), "sip:%s",
                        instance->m_thread_data.m_accounts[i].m_acc_info.m_server_host);
                }
                pjapp_transport_enum tp;
                pjapp_transport_enum * tp_ = &tp;
                switch (instance->m_thread_data.m_accounts[i].m_tp)
                {
                    case MAVC_TP_UDP:
                        tp = PJAPP_TP_UDP;
                        break;
                    case MAVC_TP_TCP:
                        tp = PJAPP_TP_TCP;
                        break;
                    case MAVC_TP_TLS:
                        tp = PJAPP_TP_TLS;
                        break;
                    default:
                        tp_ = NULL;
                }
                snprintf(server_acc.m_realm, sizeof(server_acc.m_realm), "*");
                snprintf(server_acc.m_username, sizeof(server_acc.m_username), "%s",
                    instance->m_thread_data.m_accounts[i].m_acc_info.m_username);
                snprintf(server_acc.m_password, sizeof(server_acc.m_password), "%s",
                    instance->m_thread_data.m_accounts[i].m_acc_info.m_password);
                int acc_id = PJAPP_INVALID_ID;
                pjapp_err err = pjapp_register_account(&server_acc,
                    instance->m_thread_data.m_accounts[i].m_is_default, tp_, &acc_id);
                if (PJAPP_ERR_SUCCESS != err)
                {
                    MAVC_LOGE(LOG_TAG, "Failed to register account<%s> for reason: %s.",
                        server_acc.m_acc_url, pjapp_error(err));
                }
            }
            break;
        }
        case MAVC_INTERNAL_MSG_TRY_RECYCLE_FETCH_CONFIG_THREAD: {
            mavc_t * instance = mavc_get();
            bool do_recreate = false;
            pthread_mutex_lock(&instance->m_locker);
            instance->m_thread_data.m_run = false;
            if (!instance->m_thread_data.m_do_fetch_acc)
            {
                // Do nothing.
            } else if (!instance->m_thread_data.m_in_process)
            {
                // Fetch config thread breaks from the loop, and a new fetch config request processed,
                // so join the thread and then re-create it.
                do_recreate = true;
            }
            pthread_mutex_unlock(&instance->m_locker);
            if (instance->m_tid_fetch_config > 0)
            {
                pthread_join(instance->m_tid_fetch_config, NULL);
            }
            if (do_recreate)
            {
                instance->m_thread_data.m_run = true;
                pthread_create(&instance->m_tid_fetch_config, NULL, mavc_thread_fetch_config, NULL);
            }
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

mt_status_t mavc_core_register(const mavc_config_t * config)
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

    // Set external configurations.
    mavc_get()->m_config = config;

    return mtool_module_register(&module_instance);
}

void mavc_core_unregister(void)
{
    mtool_module_unregister(&module_instance);
}

/**
 * @}
 */
