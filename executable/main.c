/**
 * @file
 * @brief Module avc main
 *
 * No more details!
 */

#include "mhub/mhub.h"
#include "../src/private.h"
#include "mavc/mavc.h"
#include <cjson/cJSON.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */

#define LOG_TAG                 "MAIN" //日志标签

#define mavc_json_get_str(json_obj, key, char_arr) do { \
    cJSON * obj_ = cJSON_GetObjectItem(json_obj, key); \
    if (NULL != obj_) \
    { \
        const char * value = cJSON_GetStringValue(obj_); \
        snprintf(char_arr, sizeof(char_arr), "%s", NULL == value ? "" : value); \
    } else \
    { \
        memset(char_arr, 0, sizeof(char_arr)); \
    } \
} while (0)

#define mavc_json_get_number(json_obj, key, val, def_val, type) do { \
    cJSON * obj_ = cJSON_GetObjectItem(json_obj, key); \
    if (NULL != obj_) \
    { \
        val = (type) cJSON_GetNumberValue(obj_); \
    } else \
    { \
        val = def_val; \
    } \
} while (0)


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

static int parse_config_file(mavc_config_t * config, const char * file)
{
    if (0 != access(file, F_OK))
    {
        MAVC_LOGE(LOG_TAG, "Configuration file<%s> not exist.", file);
        return -1;
    }
    memset(config, 0, sizeof(mavc_config_t));

    unsigned long file_size = 0;
    {
        struct stat buf;
        if (stat(file, &buf) < 0)
        {
            MAVC_LOGE(LOG_TAG, "Failed to get file size.");
            return -1;
        }
        file_size = (unsigned long) buf.st_size;
    }
    ++file_size;  // To include a '\0'
    char file_buffer[2048];
    size_t buffer_size = sizeof(file_buffer);
    char * file_content = file_buffer;
    if (file_size > sizeof(file_buffer))
    {
        file_content = malloc(file_size);
        if (NULL != file_content)
        {
            MAVC_LOGE(LOG_TAG, "Failed to alloc space for file<%s>.", file);
            return -1;
        }
        buffer_size = file_size;
    }
    int ret = -1;
    do {
        {
            FILE * fp = fopen(file, "r");
            if (NULL == fp)
            {
                break;
            }
            size_t pos = 0;
            ret = 0;
            while (pos < buffer_size)
            {
                size_t rc = fread(file_content + pos, 1, buffer_size - pos, fp);
                if (rc > 0)
                {
                    pos += rc;
                } else
                {
                    break;
                }
            }
            if (!feof(fp))
            {
                ret = -1;
            }
            fclose(fp);
            if (0 != ret)
            {
                break;
            } else
            {
                file_content[pos] = '\0';
            }
            ret = -1;
        }
        {
            cJSON * obj = cJSON_Parse(file_content);
            if (NULL == obj)
            {
                MAVC_LOGE(LOG_TAG, "Failed to parse file<%s>.", file);
                break;
            }
            {  // Parse server configurations.
                cJSON * server = cJSON_GetObjectItem(obj, "server");
                if (NULL != server)
                {
                    mavc_json_get_str(server, "username", config->m_server.m_username);
                    mavc_json_get_str(server, "password", config->m_server.m_password);
                    mavc_json_get_str(server, "server", config->m_server.m_server_host);
                    mavc_json_get_number(server, "port", config->m_server.m_port, 0, int);
                    config->m_server.m_active = true;
                } else
                {
                    config->m_server.m_active = false;
                }
            }
        }
        ret = 0;
    } while (0);
    if (buffer_size > sizeof(file_buffer))
    {
        free(file_content);
    }
    return ret;
}

int main(int argc, char *argv[], char *env[])
{
    mtool_module *instance = mavc_core_get_instance();
    int ret = 0;
    int i = 0;
    const char * config_file = NULL;
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
        } else if (strcmp(argv[i], "-c") == 0) {
            if ((++i) >= argc) {
                MAVC_LOGE(LOG_TAG, "Expect an argument for option -c.");
                print_usage(argv);
                exit(-1);
            }
            config_file = argv[i];
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

    {
        mavc_config_t config;
        if (NULL != config_file)
        {
            if (0 != parse_config_file(&config, config_file))
            {
                config_file = NULL;
            }
        }
        mavc_core_register(NULL != config_file ? &config : NULL);
    }

    while (true) {
        usleep(1000 * 1000);
    }

    return 0;
}

/**
 * @}
 */
