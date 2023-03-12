#ifndef __MAVC_JSON_H_
#define __MAVC_JSON_H_

/**
 * @file
 * @brief Module avc core
 *
 * No more details!
 */

#include "mavc_comm_objects.h"
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


char * mavc_json_mavc_call_t_2_json_obj(const mavc_call_t * call);

char * mavc_json_mavc_audio_file_ack_t_2_json_obj(const mavc_audio_file_ack_t * ack);

char * mavc_json_mavc_audio_eof_t_2_json_obj(const mavc_audio_eof_t * audio_eof);


#define mavc_json_cast_1(type, json_str, dst_obj) do { \
    mavc_json_json_obj_2_##type(json_str, dst_obj); \
} while (0)


void mavc_json_json_obj_2_mavc_call_t(const char * json_str, mavc_call_t * call);

void mavc_json_json_obj_2_mavc_audio_file_t(const char * json_str, mavc_audio_file_t * audio_file);

void mavc_json_json_obj_2_mavc_audio_volume_t(const char * json_str, mavc_audio_volume_t * audio_volume);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
