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
    const char * value = cJSON_GetStringValue(cJSON_GetObjectItem(json_obj, key)); \
    snprintf(char_arr, sizeof(char_arr), "%s", NULL == value ? "" : value); \
} while (0)


char * mavc_json_pjapp_call_t_2_json_obj(const pjapp_call_t * call)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "call_id", call->m_call_id);
    cJSON_AddStringToObject(obj, "remote_url", call->m_remote_url);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}

char * mavc_json_mavc_call_t_2_json_obj(const mavc_call_t * call)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "user_name", call->user_name);
    cJSON_AddStringToObject(obj, "remote_host", call->remote_host);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}


void mavc_json_json_obj_2_mavc_call_t(const char * json_str, mavc_call_t * call)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_str_obj_2_char_arr(obj, "user_name", call->user_name);
        json_str_obj_2_char_arr(obj, "remote_host", call->remote_host);
        cJSON_Delete(obj);
    }
}


/**
 * @}
 */
