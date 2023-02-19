/**
 * @file
 * @brief Module avc main
 *
 * No more details!
 */

#include "mhub/mhub.h"
#include "../src/private.h"
#include "mavc/mavc.h"

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */

#define LOG_TAG                 "MAIN" //日志标签

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

    mavc_core_register();

    while (true) {
        usleep(1000 * 1000);
    }

    return 0;
}

/**
 * @}
 */
