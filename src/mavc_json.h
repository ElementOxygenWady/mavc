#ifndef __MAVC_JSON_H_
#define __MAVC_JSON_H_

/**
 * @file
 * @brief PJ Application
 *
 * No more details!
 */

#include <pjapp/pjapp.h>
#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup HEADER_FILES All header files
 * @{
 */

#define mavc_json_exec_1(type, obj, str, exec_) do { \
    char * str = mavc_json_##type##_2_json_obj(obj); \
    if (NULL != str) \
    { \
        exec_; \
        free(str); \
    } \
} while (0)


char * mavc_json_pjapp_call_t_2_json_obj(const pjapp_call_t * call);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
