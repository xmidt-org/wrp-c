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
#include <sys/types.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum wrp_msg_type {
    WRP_MSG_TYPE__AUTH  = 2,
    WRP_MSG_TYPE__REQ   = 3,
    WRP_MSG_TYPE__EVENT = 4,

    WRP_MSG_TYPE__UNKNOWN = 200
};

struct wrp_auth_msg {
    int status;
};

struct wrp_timing_value {
    char *name;
    uint64_t start;
    uint64_t duration;
};

struct wrp_req_msg {
    char *transaction_uuid;
    char *source;
    char *dest;
    char **headers;                         /* NULL terminated list */
    struct wrp_timing_value *timing_values; /* NULL terminated list */
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
 *  @param bytes [out] the resulting bytes
 *
 *  @return the length of the bytes if successful or less than 1 otherwise
 */
ssize_t wrp_struct_to_bytes( const wrp_msg_t *msg, void **bytes );

/**
 *  Converts a sequence of bytes into a wrp_msg_t if possible.
 *
 *  @note If the value returned is not NULL, the resulting structure must
 *        be freed using the wrp_free_struct() function.
 *
 *  @param bytes  [in]  the sequence of bytes to process
 *  @param length [in]  the length of the bytes passed in
 *  @param msg    [out] the resulting wrp_msg_t structure if successful
 *                      unchanged otherwise
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */
ssize_t wrp_bytes_to_struct( const void *bytes, size_t length, wrp_msg_t **msg );

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
 *  Frees the wrp_msg_t structure if allocated by the wrp-c library.
 *
 *  @note Do not call this function on the wrp_msg_t structure if the wrp-c
 *        library did not create the structure!
 *
 *  @param msg [in] the wrp_msg_t structure to free
 */
void wrp_free_struct( wrp_msg_t *msg );

void example();

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */
