#ifndef __PRIVATE_H_
#define __PRIVATE_H_

/**
 * @file
 * @brief Private definitions
 *
 * Including: log wrapper, etc.
 */

#include "mavc/mavc_pub.h"
#include "mavc/mavc.h"
#if defined(ABASE_LOG_FOUND) && (ABASE_LOG_FOUND != 0)
    #include "abase/abase_log.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PRIVATE_DEF Private definitions
 * @ingroup HEADER_FILES
 * @{
 */

#ifndef DEF_WRAP
    #define DEF_WRAP                                            "\n"
#endif

#define MAVC_LOG_LABEL                                          "MAvc"

#if defined(ABASE_LOG_FOUND) && (ABASE_LOG_FOUND != 0)
    #ifdef ENABLE_DEBUG_LOG
        #define MAVC_LOGT(TAG, P, Q...)                         abase_log_trace(MAVC_LOG_LABEL, TAG, P, ##Q)
        #define MAVC_LOGD(TAG, P, Q...)                         abase_log_debug(MAVC_LOG_LABEL, TAG, P, ##Q)
    #else
        #define MAVC_LOGT(TAG, P, Q...)
        #define MAVC_LOGD(TAG, P, Q...)
    #endif
    #define MAVC_LOGI(TAG, P, Q...)                             abase_log_info(MAVC_LOG_LABEL, TAG, P, ##Q)
    #define MAVC_LOGW(TAG, P, Q...)                             abase_log_warn(MAVC_LOG_LABEL, TAG, P, ##Q)
    #define MAVC_LOGE(TAG, P, Q...)                             abase_log_error(MAVC_LOG_LABEL, TAG, P, ##Q)
    #define MAVC_LOGX(TAG, P, Q...)                             abase_log_fatal(MAVC_LOG_LABEL, TAG, P, ##Q)
#else
    #ifdef ENABLE_DEBUG_LOG
        #define MAVC_LOGT(TAG, P, Q...)                         printf("T/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
        #define MAVC_LOGD(TAG, P, Q...)                         printf("D/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
    #else
        #define MAVC_LOGT(TAG, P, Q...)
        #define MAVC_LOGD(TAG, P, Q...)
    #endif
    #define MAVC_LOGI(TAG, P, Q...)                             printf("I/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
    #define MAVC_LOGW(TAG, P, Q...)                             printf("W/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
    #define MAVC_LOGE(TAG, P, Q...)                             printf("E/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
    #define MAVC_LOGX(TAG, P, Q...)                             printf("*/" MAVC_LOG_LABEL ": [" TAG "] " P DEF_WRAP, ##Q)
#endif


#ifdef LOG_DEBUG
#ifndef LOG_TAG
#define LOG_TAG "MAVC_PRI"
#endif
#define LOG_TAG_DEBUG LOG_TAG ".Debug"
#define ASSERT_BREAK(cond) if (!(cond)) { MAVC_LOGE(LOG_TAG_DEBUG, "Error assertion: %s", #cond); break; }
#else
#define ASSERT_BREAK(cond) if (!(cond)) break
#endif


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
