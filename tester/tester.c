/**
 * @file
 * @brief Tester code
 *
 * No more details!
 */

#include "../src/private.h"

/**
 * @addtogroup TESTER_FILES All tester files
 * @{
 */

#define LOG_TAG                 "TESTER" //日志标签

int main(int argc, char *argv[], char *env[])
{
    MAVC_LOGD(LOG_TAG, "Test debug log.");
    MAVC_LOGT(LOG_TAG, "Test trace log.");
    MAVC_LOGI(LOG_TAG, "Test info  log.");
    MAVC_LOGW(LOG_TAG, "Test warn  log.");
    MAVC_LOGE(LOG_TAG, "Test error log.");

    MAVC_LOGI(LOG_TAG, "Project version: '%s', details: '%s'.", PROJECT_VERSION, PROJECT_VERSION_DETAILS);

    while (true) {
        usleep(10 * 1000);
    }

    return 0;
}

/**
 * @}
 */
