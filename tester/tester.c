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
            break;
        }
        case MSG_MAVC_CALL_OUTGOING: {
            MAVC_LOGI(LOG_TAG, "Recv MSG_MAVC_CALL_OUTGOING: %s", (char *) content);
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


#define ARG_NEXT_(cmd_, cmd_len_, process_len_) ({ \
    char * param = mavc_extract_param(cmd_, cmd_len_, &process_len_); \
    if (NULL != param) \
    { \
        cmd_len_ -= process_len_; \
        cmd_ += process_len_; \
    } \
    (param); \
})

#define CHECK_ARG_HANDLE_NULLABLE_true MAVC_LOGE(LOG_TAG, "Wrong parameters!"); continue;
#define CHECK_ARG_HANDLE_NULLABLE_false

#define CHECK_ARG_(arg_, nullable_) if (NULL == (arg_)) \
{ \
    if (NULL == (arg_)) \
    { \
        (arg_) = ""; \
    } \
    CHECK_ARG_HANDLE_NULLABLE_##nullable_ \
} else if (!strcasecmp(arg_, "--abort")) \
{ \
    MAVC_LOGI(LOG_TAG, "Aborted!"); \
    continue; \
}

#define CHECK_ABORT_ARG_(cmd_, cmd_len_, process_len_) { \
    char * param_ = NULL; \
    bool do_continue = false; \
    while (NULL != (param_ = ARG_NEXT_(cmd_, cmd_len_, process_len_))) \
    { \
        if (!strcasecmp(param_, "--abort")) \
        { \
            MAVC_LOGI(LOG_TAG, "Aborted!"); \
            do_continue = true; \
            break; \
        } \
    } \
    if (do_continue) \
    { \
        continue; \
    } \
}


// Refer from function mgwe_wifi_utils_extract_param() in mgwe.
static char * mavc_extract_param(char * raw_params, unsigned raw_params_len, unsigned * process_len)
{
    *process_len = 0;
    char * p = NULL;
    char * q = raw_params;
    bool do_continue = true;
    while (raw_params_len > 0 && do_continue)
    {
        ++*process_len;
        switch (*q)
        {
            case '\r':
            case '\n': {
                if (NULL == p)
                {
                    do_continue = false;
                    break;
                }
            }
                // Must put outside the quotes, and must not put any comments in the same line.
                // fall-through
            case ' ':
            case '\t': {
                if (NULL != p)
                {
                    *q = '\0';
                    do_continue = false;
                    break;
                }
                break;
            }
            default: {
                if (NULL == p)
                {
                    p = q;
                }
                break;
            }
        }
        --raw_params_len;
        ++q;
    }
    return p;
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
        char cmd_buffer[128] = {0};
        char * cmd = cmd_buffer;
        memset(cmd_buffer, 0, sizeof(cmd_buffer));
        char ch;
        while ('\n' != (ch = (char) getchar()) && i < (sizeof(cmd_buffer) - 1))
        {
            cmd_buffer[i++] = ch;
        }
        MAVC_LOGI(LOG_TAG, "Input command: %s", cmd);

        unsigned process_len = 0;
        unsigned cmd_len = strlen(cmd);
        char * cmd_name = mavc_extract_param(cmd, cmd_len, &process_len);
        if (NULL == cmd_name)
        {
            MAVC_LOGE(LOG_TAG, "Empty cmd.");
            continue;
        }
        cmd_len -= process_len;
        cmd += process_len;

        if (!strcasecmp(cmd_name, "Accept") || !strcmp(cmd_name, "a"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "Accept/a call-id has-audio<1|0> has-video<1|0> [--abort]");
                continue;
            }

            int call_id = atoi(param);
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool has_audio = strcmp(param, "0");
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool has_video = strcmp(param, "0");
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            cJSON * obj = cJSON_CreateObject();
            cJSON_AddNumberToObject(obj, "id", call_id);
            cJSON_AddBoolToObject(obj, "has_audio", has_audio);
            cJSON_AddBoolToObject(obj, "has_video", has_video);
            char * content = cJSON_PrintUnformatted(obj);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_ACCEPT_CALL, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
            free(content);
            cJSON_free(obj);
        } else if (!strcasecmp(cmd_name, "HangUp") || !strcmp(cmd_name, "h"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "HangUp/h call-id [--abort]");
                continue;
            }

            int call_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", call_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_HANGUP_CALL, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "MakeCall") || !strcmp(cmd_name, "m"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "MakeCall/m remote-ip has-audio<1|0> has-video<1|0> [--abort]");
                continue;
            }

            const char * remote_ip = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool has_audio = strcmp(param, "0");
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool has_video = strcmp(param, "0");
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            cJSON * obj = cJSON_CreateObject();
            cJSON_AddStringToObject(obj, "user_name", "MAVCTester");
            cJSON_AddStringToObject(obj, "remote_host", remote_ip);
            cJSON_AddBoolToObject(obj, "has_audio", has_audio);
            cJSON_AddBoolToObject(obj, "has_video", has_video);
            char * content = cJSON_PrintUnformatted(obj);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_MAKE_CALL, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
            free(content);
            cJSON_free(obj);
        } else if (!strcasecmp(cmd_name, "Cancel") || !strcmp(cmd_name, "c"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "Cancel/c call-id [--abort]");
                continue;
            }

            int call_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", call_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_CANCEL_MAKE_CALL, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "Reject") || !strcmp(cmd_name, "r"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "Reject/r call-id [--abort]");
                continue;
            }

            int call_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", call_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_REJECT_CALL, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "Play") || !strcmp(cmd_name, "p"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "Play/p file loop<0|1> [--abort]");
                continue;
            }

            const char * file = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool loop = strcmp(param, "0");
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"file\": \"%s\", \"loop\": %d}", file, loop ? 1 : 0);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_START_PLAY_AUDIO, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);

                mtool_module_message_holder_destroy(holder);
                holder = NULL;
            }
        } else if (!strcasecmp(cmd_name, "StopPlay") || !strcmp(cmd_name, "sp"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "StopPlay/sp play-id [--abort]");
                continue;
            }

            int play_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", play_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_STOP_PLAY_AUDIO, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "PausePlay") || !strcmp(cmd_name, "pp"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "PausePlay/pp play-id [--abort]");
                continue;
            }

            int play_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", play_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_PAUSE_PLAY_AUDIO, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "ResumePlay") || !strcmp(cmd_name, "rp"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "ResumePlay/rp play-id [--abort]");
                continue;
            }

            int play_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d}", play_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_RESUME_PLAY_AUDIO, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "VolumeDown") || !strcmp(cmd_name, "vd"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "VolumeDown/vd play-id [--abort]");
                continue;
            }

            int play_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            if (g_current_volume > 0)
            {
                --g_current_volume;
            }

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d, \"volume\": %d}", play_id, g_current_volume);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_VOLUME, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "VolumeUp") || !strcmp(cmd_name, "vu"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "VolumeUp/vu play-id [--abort]");
                continue;
            }

            int play_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            if (g_current_volume < 10)
            {
                ++g_current_volume;
            }

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"id\": %d, \"volume\": %d}", play_id, g_current_volume);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_VOLUME, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "RecordBegin") || !strcmp(cmd_name, "rb"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "RecordBegin/rb file source<MIC|CALL> [--abort]");
                continue;
            }

            const char * file = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            const char * source_str = param;
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            if (!strcasecmp(source_str, "MIC") ||
                !strcasecmp(source_str, "CALL"))
            {
                // Do nothing.
            } else
            {
                MAVC_LOGE(LOG_TAG, "Invalid source: %s", source_str);
                continue;
            }

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"file\": \"%s\", \"sources\": \"%s\"}", file, source_str);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_START_RECORD, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "RecordEnd") || !strcmp(cmd_name, "re"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, false);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "RecordEnd/re [--abort]");
                continue;
            }
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_BINARY_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_STOP_RECORD, 0, 0, NULL, 0, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd_name, "RegisterServer") || !strcmp(cmd_name, "rs"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "RegisterServer/rs server_host port tranport_type<UDP|TCP|TLS|DEF>" \
                    " username password default<0|1> [--abort]");
                continue;
            }

            const char * server_host = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            int port = atoi(param);
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            const char * tp_str = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            const char * username = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            const char * password = param;
            param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            bool is_default = 0 != atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);


            int tp = 0;
            if (!strcasecmp(tp_str, "UDP"))
            {
                tp = 1;
            } else if (!strcasecmp(tp_str, "TCP"))
            {
                tp = 2;
            } else if (!strcasecmp(tp_str, "TLS"))
            {
                tp = 3;
            }

            char content[128];
            snprintf(content, sizeof(content), "{\"username\": \"%s\", \"password\": \"%s\", " \
                "\"server_host\": \"%s\", \"port\": %d, \"is_default\": %s, \"transport\": %d}",
                username, password, server_host, port, is_default ? "true" : "false", tp);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_REGISTER_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd_name, "UnRegisterServer") || !strcmp(cmd_name, "urs"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "UnRegisterServer/urs acc_id [--abort]");
                continue;
            }

            int acc_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"account_id\": %d}", acc_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_UNREGISTER_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if(!strcasecmp(cmd_name, "GetAccountList") || !strcmp(cmd_name, "gal"))
        {
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, false);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "GetAccountList/gal [--abort]");
                continue;
            }
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

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
            char * param = ARG_NEXT_(cmd, cmd_len, process_len);
            CHECK_ARG_(param, true);
            if (!strcasecmp(param, "--help") || !strcasecmp(param, "-h"))
            {
                MAVC_LOGI(LOG_TAG, "SetDefaultAccount/sda acc_id [--abort]");
                continue;
            }

            int acc_id = atoi(param);
            CHECK_ABORT_ARG_(cmd, cmd_len, process_len);

            char content[64] = {0};
            snprintf(content, sizeof(content), "{\"account_id\": %d}", acc_id);
            mtool_module_message_holder * holder = NULL;
            mtool_module_send_reliable(MTOOL_MODULE_MESSAGE_JSON_CONTENT,
                MODULE_NAME, -1, MTOOL_MODULE_AVC_NAME, -1,
                NULL, MSG_MAVC_SET_DEFAULT_ACCOUNT, 0, 0, content, strlen(content) + 1, 2000, &holder);
            if (NULL != holder)
            {
                MAVC_LOGI(LOG_TAG, "Recv: %s", (char *) holder->content);
                mtool_module_message_holder_destroy(holder);
            }
        } else if (!strcasecmp(cmd_name, "quit") || !strcmp(cmd_name, "q"))
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
