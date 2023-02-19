#ifndef __MAVC_H_
#define __MAVC_H_

/**
 * @file
 * @brief Module avc
 *
 * No more details!
 */

#include "mavc_pub.h"
#include "mtool/mtool_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup HEADER_FILES All header files
 * @{
 */

mtool_module *mavc_core_get_instance(void);

mt_status_t mavc_core_register(void);

void mavc_core_unregister(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
