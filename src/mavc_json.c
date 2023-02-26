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

/**
 * @addtogroup SOURCE_FILES All source files
 * @{
 */


char * mavc_json_pjapp_call_t_2_json_obj(const pjapp_call_t * call)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "call_id", call->m_call_id);
    cJSON_AddStringToObject(obj, "remote_url", call->m_remote_url);
    char * str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return str;
}


/**
 * @}
 */
