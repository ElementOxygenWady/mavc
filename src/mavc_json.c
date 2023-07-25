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

#define json_get_bool(json_obj, key, val, def_val) do { \
    cJSON * obj_ = cJSON_GetObjectItem(json_obj, key); \
    if (NULL != obj_) \
    { \
        val = cJSON_IsTrue(obj_) ? true : false; \
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


static int mavc_json_get_multi_values(const char * src,
    void (* on_find_value)(const char * value, void * user_data), void * user_data)
{
    char sources_str[64];
    snprintf(sources_str, sizeof(sources_str), "%s", src);
    char * p = sources_str;
    char * q = sources_str;
    int last_split = 1;
    int no_err = 1;
    int may_err = 0;
    while ('\0' != *q && q < sources_str + sizeof(sources_str) - 1 && no_err)
    {
        switch (*q)
        {
            case '|': {
                if (last_split)
                {
                    no_err = 0;
                    break;
                }
                last_split = 1;
                may_err = 0;
                *q = '\0';
                if (q > p)  // Has content
                {
                    on_find_value(p, user_data);
                }
                ++q;
                p = q;
                break;
            }
            case ' ':
            case '\t': {
                *q = '\0';
                if (!last_split)
                {
                    may_err = 1;  // If the next char is '|', it is not an error, otherwise, it is an error.
                    ++q;
                    break;
                }
                if (q > p)  // Has content
                {
                    on_find_value(p, user_data);
                }
                ++q;
                p = q;
                break;
            }
            default: {
                if (may_err)
                {
                    no_err = 0;
                    break;
                }
                ++q;
                last_split = 0;
                break;
            }
        }
    }
    if (no_err && last_split)
    {
        no_err = 0;
    }
    if (q > p && no_err)  // Has content
    {
        *q = '\0';
        on_find_value(p, user_data);
    }
    return no_err ? 0 : -1;
}


void mavc_json_json_obj_2_mavc_call_t(const char * json_str, mavc_call_t * call)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_get_number(obj, "id", call->id, -1, int);
        json_str_obj_2_char_arr(obj, "user_name", call->user_name);
        json_str_obj_2_char_arr(obj, "remote_host", call->remote_host);
        json_get_bool(obj, "has_audio", call->has_audio, true);
        json_get_bool(obj, "has_video", call->has_video, true);
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

static void mavc_json_on_find_audio_source(const char * source, void * user_data)
{
    unsigned * sources = (unsigned *) user_data;
    if (!strcasecmp(source, "MIC"))
    {
        sources[0] |= PJAPP_REC_SOURCE_AUDIO_MIC;
    } else if (!strcasecmp(source, "CALL"))
    {
        sources[0] |= PJAPP_REC_SOURCE_AUDIO_CALL;
    }
}

void mavc_json_json_obj_2_mavc_record_t(const char * json_str, mavc_record_t * record)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_str_obj_2_char_arr(obj, "file", record->filename);
        {
            unsigned sources[1] = { 0 };
            char sources_str[64];
            json_str_obj_2_char_arr(obj, "sources", sources_str);
            mavc_json_get_multi_values(sources_str, mavc_json_on_find_audio_source, sources);
            record->sources = sources[0];
        }
        cJSON_Delete(obj);
    }
}

void mavc_json_json_obj_2_mavc_server_account_t(const char * json_str, mavc_server_account_t * server_acc)
{
    cJSON * obj = cJSON_Parse(json_str);
    if (NULL != obj)
    {
        json_str_obj_2_char_arr(obj, "username", server_acc->m_acc_info.m_username);
        json_str_obj_2_char_arr(obj, "password", server_acc->m_acc_info.m_password);
        json_str_obj_2_char_arr(obj, "server_host", server_acc->m_acc_info.m_server_host);
        json_get_number(obj, "port", server_acc->m_acc_info.m_port, 5060, int);
        json_get_number(obj, "transport", server_acc->m_tp, MAVC_TP_AUTO, mavc_transport_e);
        json_get_bool(obj, "is_default", server_acc->m_is_default, true);
        cJSON_Delete(obj);
    }
}


/**
 * @}
 */
