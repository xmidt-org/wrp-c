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
    WRP_MSG_TYPE__AUTH          = 2,
    WRP_MSG_TYPE__REQ           = 3,
    WRP_MSG_TYPE__EVENT         = 4,
    WRP_MSG_TYPE__CREATE        = 5,
    WRP_MSG_TYPE__RETREIVE      = 6,
    WRP_MSG_TYPE__UPDATE        = 7,
    WRP_MSG_TYPE__DELETE        = 8,
    WRP_MSG_TYPE__SVC_REGISTRATION      = 9,
    WRP_MSG_TYPE__SVC_ALIVE     = 10,
    WRP_MSG_TYPE__UNKNOWN = 200
};

enum wrp_format {
    WRP_BYTES = 0,
    WRP_BASE64 = 1,
    WRP_STRING = 2
};

enum wrp_device_id_element {
    WRP_ID_ELEMENT__SCHEME      = 0,
    WRP_ID_ELEMENT__ID          = 1,
    WRP_ID_ELEMENT__SERVICE     = 2,
    WRP_ID_ELEMENT__APPLICATION = 3
};

enum wrp_token_name {
    SOURCE = 0,
    DEST = 1
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


typedef struct headers_struct {
    size_t count;
    // Flexible Array Must be the last element
    char *headers[];
} headers_t;

typedef struct partners_struct {
    size_t count;
    char *partner_ids[];
} partners_t;

struct data {
    char *name;
    char *value;
};

typedef struct data_struct {
    size_t count;
    struct data *data_items;
} data_t;

struct wrp_req_msg {
    char *transaction_uuid;
    char *content_type;
    char *source;
    char *dest;
    partners_t *partner_ids;
    headers_t *headers;                         /* NULL terminated list */
    data_t *metadata;
    bool include_spans;
    struct money_trace_spans spans;
    void *payload;
    size_t payload_size;
};

struct wrp_event_msg {
    char *content_type;
    char *source;
    char *dest;
    partners_t *partner_ids;
    headers_t *headers;                         /* NULL terminated list */
    data_t *metadata;
    void *payload;
    size_t payload_size;
};

struct wrp_crud_msg {
    char *content_type;
    char *transaction_uuid;
    char *source;
    char *dest;
    partners_t *partner_ids;
    headers_t *headers;                         /* NULL terminated list */
    data_t *metadata;
    bool include_spans;
    struct money_trace_spans spans;
    int status;
    int rdr;
    char *path;
    void *payload;
    size_t payload_size;
};

struct wrp_svc_registration_msg {
    char *service_name;
    char *url;
};

typedef struct {
    enum wrp_msg_type msg_type;

    union {
        struct wrp_auth_msg  auth;
        struct wrp_req_msg   req;
        struct wrp_event_msg event;
        struct wrp_crud_msg crud;
        struct wrp_svc_registration_msg reg;
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

/**
 *  Encode/pack only metadata from wrp_msg_t structure.
 *
 *  @note Do not call free of output data in failure case!
 *
 *  @param msg [in] packData the data_t structure to pack/encode
 *  @param msg [out] the encoded output
 *  @return encoded buffer size or less than 1 in failure case
 */

ssize_t wrp_pack_metadata( const data_t *packData, void **data );

/**
 * @brief appendEncodedData function to append two encoded buffer and change MAP size accordingly.
 *
 * @note appendEncodedData function allocates memory for buffer, caller needs to free the buffer(appendData)in
 * both success or failure case. use wrp_free_struct() for free
 *
 * @param[in] encodedBuffer msgpack object (first buffer)
 * @param[in] encodedSize is size of first buffer
 * @param[in] metadataPack msgpack object (second buffer)
 * @param[in] metadataSize is size of second buffer
 * @param[out] appendData final encoded buffer after append
 * @return  appended total buffer size or less than 1 in failure case
 */

size_t appendEncodedData( void **appendData, void *encodedBuffer, size_t encodedSize, void *metadataPack, size_t metadataSize );


/**
 *  Find the destination of a wrp_msg_t 
 *
 *  @param msg [in] the wrp_msg_t structure to examine
 *  @return pointer to destination, or NULL if there is no
 *    destination for this message type
 */
const char *wrp_get_msg_dest( const wrp_msg_t *wrp_msg );

/**
 *  Find the source of a wrp_msg_t
 *
 *  @param msg [in] the wrp_msg_t structure to examine
 *  @return pointer to source, or NULL if there is no
 *    source for this message type
 */
const char *wrp_get_msg_source( const wrp_msg_t *wrp_msg );

/**
 *  Find an element of the source or destination of a wrp_msg_t
 *
 *  @note Returned memory must be freed using free().
 *
 * @param [in] the element requested to parse
 * @param msg [in] the wrp_msg_t structure to examine
 * @param [in] the wrp_token source or destination to examine
 *
 *  @return pointer to the element requested, or NULL otherwise
 */
char *wrp_get_msg_element( const enum wrp_device_id_element element,
                                const wrp_msg_t *wrp_msg, const enum wrp_token_name wrp_token );

/**
 *  Check to see if the service matches based on the specified device_id.
 *
 *  @param [in] service   the service name check the device_id for
 *  @param [in] device_id the device-id string to examine looking for the service
 *
 *  @return 0 if there is a match, -1 otherwise.
 */
int wrp_does_service_match( const char *service, const char *device_id );
#endif
