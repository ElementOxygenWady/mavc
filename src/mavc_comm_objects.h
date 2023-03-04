#ifndef __MAVC_COMM_OBJECTS_H_
#define __MAVC_COMM_OBJECTS_H_

/**
 * @file
 * @brief Module avc core
 *
 * No more details!
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup HEADER_FILES All header files
 * @{
 */

typedef struct mavc_call_t
{
    int id;
    char user_name[16];
    char remote_host[32];
} mavc_call_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
