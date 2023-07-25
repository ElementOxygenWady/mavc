#ifndef __MAVC_PUB_H_
#define __MAVC_PUB_H_

/**
 * @file
 * @brief Public definitions
 *
 * Including: data types, constant values, common functions, etc.
 */

#include <stddef.h>     //size_t, NULL
#include <stdint.h>     //int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
#include <stdbool.h>    //bool, true, false
#include <stdlib.h>     //malloc, free
#include <string.h>     //strcpy, strcmp, memcpy, memcmp
#include <stdio.h>      //fopen, fclose
#include <errno.h>      //errno
#include <unistd.h>     //sleep, usleep

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PUBLIC_DEF Public definitions
 * @ingroup HEADER_FILES
 * @{
 */


enum mavc_transport_e;
struct mavc_config_server_t;
struct mavc_config_t;
struct mavc_server_account_t;

typedef enum mavc_transport_e mavc_transport_e;
typedef struct mavc_config_server_t mavc_config_server_t;
typedef struct mavc_config_t mavc_config_t;
typedef struct mavc_server_account_t mavc_server_account_t;


enum mavc_transport_e
{
    MAVC_TP_AUTO,
    MAVC_TP_UDP,
    MAVC_TP_TCP,
    MAVC_TP_TLS,
};

struct mavc_config_server_t
{
    char m_username[16];
    char m_password[32];  // @todo: Encrypt it.
    char m_server_host[16];
    int m_port;
    bool m_active;
};

struct mavc_config_t
{
    mavc_config_server_t m_server;
};

struct mavc_server_account_t
{
    mavc_config_server_t m_acc_info;
    mavc_transport_e m_tp;
    char m_is_default;
    char m_reserved[3];
};


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
