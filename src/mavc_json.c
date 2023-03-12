/**
 * @file
 * @brief Module avc core
 *
 * No more details!
 */

#include "private.h"
#include "mavc_json.h"
#include <e2str/e2str_module_message.h>
#include <pjapp/pjapp.h>
#include <stdio.h>

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */


#define json_str_obj_2_char_arr(json_obj, key, char_arr) do { \
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

#define json_get_number(json_obj, key, val, def_val, type) do { \
    cJSON * obj_ = cJSON_GetObjectItem(json_obj, key); \
    if (NULL != obj_) \
    { \
        val = (type) cJSON_GetNumberValue(obj_); \
    } else \
    { \
        val = def_val; \
    } \
} while (0)


char * mavc_json_mavc_call_t_2_json_obj(const mavc_call_t * call)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", call->id);
    cJSON_AddStringToObject(obj, "user_name", call->user_name);
    cJSON_AddStringToObject(obj, "remote_host", call->remote_host);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}

char * mavc_json_mavc_audio_file_ack_t_2_json_obj(const mavc_audio_file_ack_t * ack)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", ack->id);
    cJSON_AddNumberToObject(obj, "status", ack->status);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}

char * mavc_json_mavc_audio_eof_t_2_json_obj(const mavc_audio_eof_t * audio_eof)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", audio_eof->id);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}


void mavc_json_json_obj_2_mavc_call_t(const char * json_str, mavc_call_t * call)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_get_number(obj, "id", call->id, -1, int);
        json_str_obj_2_char_arr(obj, "user_name", call->user_name);
        json_str_obj_2_char_arr(obj, "remote_host", call->remote_host);
        cJSON_Delete(obj);
    }
}

void mavc_json_json_obj_2_mavc_audio_file_t(const char * json_str, mavc_audio_file_t * audio_file)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_get_number(obj, "id", audio_file->id, -1, int);
        json_str_obj_2_char_arr(obj, "file", audio_file->filename);
        json_get_number(obj, "loop", audio_file->loop, 0, int);
        cJSON_Delete(obj);
    }
}

void mavc_json_json_obj_2_mavc_audio_volume_t(const char * json_str, mavc_audio_volume_t * audio_volume)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_get_number(obj, "id", audio_volume->id, -1, int);
        json_get_number(obj, "volume", audio_volume->volume, 5, int);
        cJSON_Delete(obj);
    }
}


/**
 * @}
 */
