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
    int status;
    char user_name[64];
    char remote_host[64];
    char status_desc[32];
    bool has_audio;
    bool has_video;
    char reserved[2];
} mavc_call_t;

typedef struct mavc_audio_file_t
{
    int id;
    int loop;
    char filename[64];
} mavc_audio_file_t;

typedef struct mavc_audio_file_ack_t
{
    int status;
    int id;
} mavc_audio_file_ack_t;

typedef struct mavc_audio_volume_t
{
    int id;
    int volume;
} mavc_audio_volume_t;

typedef struct mavc_audio_eof_t
{
    int id;
} mavc_audio_eof_t;

typedef struct mavc_record_t
{
    char filename[64];
    unsigned sources;
} mavc_record_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
