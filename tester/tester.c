/**
 * @file
 * @brief Tester code
 *
 * No more details!
 */

#include "../src/private.h"
#include "mhub/mhub.h"
#include "../src/private.h"
#include "mavc/mavc.h"
#include <e2str/e2str_module_message.h>
#include <cjson/cJSON.h>

/**
 * @addtogroup TESTER_FILES All tester files
 * @{
 */

#define LOG_TAG                 "TESTER" //日志标签

#define MODULE_NAME MTOOL_MODULE_GUI_NAME
#define CALL_IP "192.168.31.142"
// #define CALL_IP "192.168.31.90"

static int g_call_id = -1;
static int g_play_id = -1;
static int g_current_volume = 5;


static mt_status_t module_load(mtool_module *module)
{
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
        case MSG_MAVC_INCOMING_CALL: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_INCOMING_CALL: %s", (char *) content);
            cJSON * obj = cJSON_Parse((char *) content);
            g_call_id = (int) cJSON_GetNumberValue(cJSON_GetObjectItem(obj, "id"));
            cJSON_Delete(obj);
            break;
        }
        case MSG_MAVC_CALL_OUTGOING: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_OUTGOING: %s", (char *) content);
            cJSON * obj = cJSON_Parse((char *) content);
            g_call_id = (int) cJSON_GetNumberValue(cJSON_GetObjectItem(obj, "id"));
            cJSON_Delete(obj);
            break;
        }
        case MSG_MAVC_CALL_CONFIRMED: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_CONFIRMED: %s", (char *) content);
            break;
        }
        case MSG_MAVC_CALL_DISCONNECTED: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_DISCONNECTED: %s", (char *) content);
            break;
        }
        case MSG_MAVC_CALL_CANCELLED: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_CANCELLED: %s", (char *) content);
            break;
        }
        case MSG_MAVC_CALL_REJECTED: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_REJECTED: %s", (char *) content);
            break;
        }
        case MSG_MAVC_AUDIO_PLAY_FINISHED: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_AUDIO_PLAY_FINISHED: %s", (char *) content);
            break;
        }
        case MSG_MAVC_NOTIFY_REGISTER_STATUS: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_NOTIFY_REGISTER_STATUS: %s", (char *) content);
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
        .name = {MODULE_NAME},
        .mode = MTOOL_MODULE_ATTACH_THREAD | MTOOL_MODULE_ENABLE_RELIABLE_SENDER,
        .transport = {
            .type = MTOOL_MODULE_TRANSPORT_TYPE_UDP,
            .udp = {
                .port = MTOOL_MODULE_AVC_UDP_PORT + 1,
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

mt_status_t mavc_tester_register(void)
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

void mavc_tester_unregister(void)
{
    mtool_module_unregister(&module_instance);
}

static int print_usage(char *argv[])
{
    printf("Usage:\n");
    printf("%s [--help] [-h] [-l <log level>] [-o <log file>] [-p] [-e <extra component>]\n", argv[0]);
    printf("Examples:\n");
    printf("%s -l \"INFO\" -o \"/tmp/tmp.log\" -p -e \"MHUB\"\n", argv[0]);
    printf("    -l, Set log level name, the value supports TRACE,DEBUG,INFO,WARN,ERROR,FATAL\n");
    printf("    -o, Change log file name, its directory must exist\n");
    printf("    -p, Print log to screen\n");
    printf("    -e, Execute extra components, built-in option includes MHUB\n");
    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    MAVC_LOGD(LOG_TAG, "Test debug log.");
    MAVC_LOGT(LOG_TAG, "Test trace log.");
    MAVC_LOGI(LOG_TAG, "Test info  log.");
    MAVC_LOGW(LOG_TAG, "Test warn  log.");
    MAVC_LOGE(LOG_TAG, "Test error log.");

    MAVC_LOGI(LOG_TAG, "Project version: '%s', details: '%s'.", PROJECT_VERSION, PROJECT_VERSION_DETAILS);

    mtool_module *instance = mavc_core_get_instance();
    int ret = 0;
    int i = 0;
    bool exe_mhub = false;

    ret = abase_log_init(DEFAULT_LOG_FILE, DEFAULT_LOG_FILE_MAX_SIZE, DEFAULT_LOG_FILE_MAX_COUNT,
            ABASE_LOG_DEFAULT_BUFFSIZE, ABASE_LOG_DEFAULT_OPTION);
    if (ret != 0) {
        MAVC_LOGE(LOG_TAG, "Failed to initialize ABase log.");
    }

    abase_log_set_level(ABASE_LOG_DEFAULT_LEVEL);

    for (i = 1; i < argc; i++) {
        if (strcasecmp(argv[i], "--help") == 0) {
            print_usage(argv);
            exit(0);
        } else if (strcasecmp(argv[i], "-h") == 0) {
            print_usage(argv);
            exit(0);
        } else if (strcmp(argv[i], "-l") == 0) {
            if ((++i) >= argc)
                break;
            abase_log_set_level_by_name(argv[i]);
        } else if (strcmp(argv[i], "-o") == 0) {
            if ((++i) >= argc)
                break;
            abase_log_set_filename(argv[i]);
        } else if (strcmp(argv[i], "-p") == 0) {
            abase_log_set_print_screen(true);
        } else if (strcmp(argv[i], "-e") == 0) {
            if ((++i) >= argc)
                break;
            if (strcasecmp(argv[i], "mhub") == 0) {
                exe_mhub = true;
            }
        } else {
            MAVC_LOGE(LOG_TAG, "Unknown argument '%s'.", argv[i]);
            print_usage(argv);
            exit(-1);
        }
    }

    MAVC_LOGI(LOG_TAG, "Project version: '%s', details: '%s'.", PROJECT_VERSION, PROJECT_VERSION_DETAILS);

    if (exe_mhub) {
        instance->attr.mode |= MTOOL_MODULE_CREATE_TRANSPORT;
                //如果不创建新的TRANSPORT, 在MHub的缓存会使用错误的端口
                //强制修改模式是临时的解决方法, 彻底解决该问题需要修改MTool和MHub
        mhub_core_register();
    }

    mavc_tester_register();

    while (true) {
        unsigned i = 0;
        char cmd[16] = {0};
        memset(cmd, 0, sizeof(cmd));
        char ch;
        while ('\n' != (ch = getchar()) && i < (sizeof(cmd) - 1))
        {
            cmd[i++] = ch;
        }
        MAVC_LOGD(LOG_TAG, "Input command: %s", cmd);
        if (!strcasecmp(cmd, "Accept") || !strcmp(cmd, "a"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_ACCEPT_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "AcceptAudio") || !strcmp(cmd, "aa"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d, \"has_audio\": true, \"has_video\": false}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_ACCEPT_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "AcceptVideo") || !strcmp(cmd, "av"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d, \"has_audio\": false, \"has_video\": true}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_ACCEPT_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "HangUp") || !strcmp(cmd, "h"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_HANGUP_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "MakeCall") || !strcmp(cmd, "m"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"user_name\": \"MAVCTester\", \"remote_host\": \"" CALL_IP "\"}");
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_MAKE_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "MakeCallAudio") || !strcmp(cmd, "ma"))
        {
            char content[128] = {0};
            snprintf(content, sizeof(content), "{\"user_name\": \"MAVCTester\", \"remote_host\": \"" CALL_IP
                "\", \"has_audio\": true, \"has_video\": false}");
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_MAKE_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "MakeCallVideo") || !strcmp(cmd, "mv"))
        {
            char content[128] = {0};
            snprintf(content, sizeof(content), "{\"user_name\": \"MAVCTester\", \"remote_host\": \"" CALL_IP
                "\", \"has_audio\": false, \"has_video\": true}");
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_MAKE_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "Cancel") || !strcmp(cmd, "c"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_CANCEL_MAKE_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "Reject") || !strcmp(cmd, "r"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_call_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_REJECT_CALL, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "Play") || !strcmp(cmd, "p"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"file\": \"/customer/nfs/1.wav\", \"loop\": 0}");
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_START_PLAY_AUDIO, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                cJSON * obj = cJSON_Parse((char *) holder->content);
                g_play_id = (int) cJSON_GetNumberValue(cJSON_GetObjectItem(obj, "id"));
                cJSON_Delete(obj);

                mtool_module_message_holder_destroy(holder);
                holder = NULL;
            }
        } else if (!strcasecmp(cmd, "PausePlay") || !strcmp(cmd, "pp"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_play_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_PAUSE_PLAY_AUDIO, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "ResumePlay") || !strcmp(cmd, "rp"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_play_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_RESUME_PLAY_AUDIO, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "StopPlay") || !strcmp(cmd, "sp"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", g_play_id);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_STOP_PLAY_AUDIO, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "Down") || !strcmp(cmd, "d"))
        {
            char content[64] = {0};
            if (g_current_volume > 0)
            {
                --g_current_volume;
            }
            snprintf(content, sizeof(content), "{\"id\": %d, \"volume\": %d}", g_play_id, g_current_volume);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_VOLUME, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "Up") || !strcmp(cmd, "u"))
        {
            char content[64] = {0};
            if (g_current_volume < 10)
            {
                ++g_current_volume;
            }
            snprintf(content, sizeof(content), "{\"id\": %d, \"volume\": %d}", g_play_id, g_current_volume);
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_VOLUME, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "RecordBegin") || !strcmp(cmd, "rb"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"file\": \"/customer/nfs/record.wav\", \"sources\": \"MIC\"}");
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_START_RECORD, 0, 0, content, strlen(content) + 1);
        } else if (!strcasecmp(cmd, "RecordEnd") || !strcmp(cmd, "re"))
        {
            mtool_module_send_nonblock(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_STOP_RECORD, 0, 0, NULL, 0);
        } else if(!strcasecmp(cmd, "RegisterServer") || !strcmp(cmd, "rs"))
        {
            char content[128] = {0};
            snprintf(content, sizeof(content), "{\"username\": \"100025\", \"password\": \"h12345\", " \
                "\"server_host\": \"120.76.99.131\", \"port\": 25099, \"is_default\": true}");
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_REGISTER_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd, "UnRegisterServer") || !strcmp(cmd, "urs"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"account_id\": 2}");
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_UNREGISTER_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd, "GetAccountList") || !strcmp(cmd, "gal"))
        {
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_GET_ACCOUNT_LIST, 0, 0, NULL, 0, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd, "SetDefaultAccount") || !strcmp(cmd, "sda"))
        {
            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"account_id\": 2}");
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_DEFAULT_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd, "quit") || !strcmp(cmd, "q"))
        {
            break;
        }
        usleep(10 * 1000);
    }

    return 0;
}

/**
 * @}
 */
