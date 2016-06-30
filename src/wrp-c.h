/**
 * Copyright 2016 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef __WRP_C_H__
#define __WRP_C_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum wrp_msg_type {
    WRP_MSG_TYPE__AUTH    = 2,
    WRP_MSG_TYPE__REQ     = 3,
    WRP_MSG_TYPE__EVENT   = 4,

    WRP_MSG_TYPE__UNKNOWN = 200
};

enum wrp_format {
    WRP_BYTES,
    WRP_BASE64,
    WRP_STRING
};

struct wrp_auth_msg {
    int status;
};

struct money_trace_span {
    char *name;
    uint64_t start;     /* Start time in microseconds from 1/1/1970. */
    uint32_t duration;  /* Duration in microseconds from the start time. */
};

struct money_trace_spans {
    struct money_trace_span *spans;
    size_t count;
};

struct wrp_req_msg {
    char *transaction_uuid;
    char *source;
    char *dest;
    char **headers;                         /* NULL terminated list */
    bool include_spans;
    struct money_trace_spans spans;
    void *payload;
    size_t payload_size;
};

struct wrp_event_msg {
    char *source;
    char *dest;
    char **headers;                         /* NULL terminated list */
    void *payload;
    size_t payload_size;
};

typedef struct {
    enum wrp_msg_type msg_type;

    union {
        struct wrp_auth_msg  auth;
        struct wrp_req_msg   req;
        struct wrp_event_msg event;
    } u;
} wrp_msg_t;

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  Converts one of the wrp data structures into the sequence of bytes
 *  representation.
 *
 *  @note If the value returned is greater than 0, the value pointed at by
 *        bytes must be freed using free() by the caller.
 *
 *  @param msg   [in]  the wrp_msg_t structure to convert
 *  @param fmt   [in]  the format the output should be converted into
 *  @param bytes [out] the resulting bytes (not changed on error)
 *
 *  @return the length of the bytes if successful or less than 1 otherwise
 */
ssize_t wrp_struct_to( const wrp_msg_t *msg, const enum wrp_format fmt,
                       void **bytes );


/**
 *  Converts a sequence of bytes into a wrp_msg_t if possible.
 *
 *  @note If the value returned is not NULL, the resulting structure must
 *        be freed using the wrp_free_struct() function.
 *
 *  @note fmt may only be: WRP_BYTES or WRP_BASE64.
 *
 *  @param bytes  [in]  the sequence of bytes to process
 *  @param length [in]  the length of the bytes passed in
 *  @param fmt    [in]  the format the input should be converted from
 *  @param msg    [out] the resulting wrp_msg_t structure if successful
 *                      unchanged otherwise (not changed on error)
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */
ssize_t wrp_to_struct( const void *bytes, const size_t length,
                       const enum wrp_format fmt,
                       wrp_msg_t **msg );


/**
 *  Converts a wrp_msg_t structure into a printable string.
 *
 *  @note If the value returned is greater than 0, the value pointed at by
 *        bytes must be freed using free() by the caller.
 *
 *  @param msg [in] the wrp_msg_t structure to convert
 */
char* wrp_struct_to_string( const wrp_msg_t *msg );


/**
 *  Free the wrp_msg_t structure if allocated by the wrp-c library.
 *
 *  @note Do not call this function on the wrp_msg_t structure if the wrp-c
 *        library did not create the structure!
 *
 *  @param msg [in] the wrp_msg_t structure to free
 */
void wrp_free_struct( wrp_msg_t *msg );

#endif
