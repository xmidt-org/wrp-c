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
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <trower-base64/base64.h>

#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/**
 *  Free only if allocated .
 */
#define WRP_FREE(__x__) if(__x__ != NULL) { free((void*)(__x__)); __x__ = NULL;} else {printf("Trying to free null pointer\n");}
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static const char const *__empty_list               = "''";
static const char const * WRP_MSG_TYPE              = "msg_type";
static const char const * WRP_SOURCE                = "source";
static const char const * WRP_DESTINATION           = "dest";
static const char const * WRP_TRANSACTION_ID        = "transaction_uuid";
static const char const * WRP_HEADERS               = "headers";
static const char const * WRP_PAYLOAD               = "payload";
static const char const * WRP_TIMING_VALUES         = "timing_values";
static const char const * WRP_INCLUDE_TIMING_VALUES = "include_timing_values";
static const char const * WRP_STATUS                = "status";
static const int const WRP_MAP_SIZE                 = 4; // mandatory msg_type,source,dest,payload


/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth,
                                            char **bytes );
static ssize_t __wrp_req_struct_to_string( const struct wrp_req_msg *req, char **bytes );
static ssize_t __wrp_event_struct_to_string( const struct wrp_event_msg *event,
                                             char **bytes );
static char* __get_header_string( char **headers );
static char* __get_spans_string( const struct money_trace_spans *spans );
static ssize_t __wrp_pack_structure( int msg_type, char* source, char* dest,
                                     char* transaction_uuid, bool includeTimingValues,
                                     const struct money_trace_spans *timeSpan,
                                     char* payload, size_t payload_size,
                                     char **headers, char **data );
static ssize_t __wrp_base64_to_struct( const void *base64_data, const size_t base64_size,
                                       wrp_msg_t **msg_ptr );
static ssize_t __wrp_bytes_to_struct( const void *bytes, const size_t length,
                                      wrp_msg_t **msg_ptr );
static void decodeRequest( msgpack_object deserialized, int *msgType, char** source_ptr,
                           char** dest_ptr, char** transaction_id_ptr, char*** headers,
                           int *statusValue,
                           char** payload_ptr, bool *includeTimingValues );
static char* getKey_MsgtypeStr( const msgpack_object key, const size_t keySize,
                                char* keyString );
static char* getKey_MsgtypeBin( const msgpack_object key, const size_t binSize,
                                char* keyBin );


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/* See wrp-c.h for details. */
ssize_t wrp_struct_to( const wrp_msg_t *msg, const enum wrp_format fmt, void **bytes )
{
    char *data;
    ssize_t rv;

    if( NULL == msg ) {
        return -1;
    }

    switch( fmt ) {
        case WRP_BYTES:
            rv = __wrp_struct_to_bytes( msg, &data );
            break;

        case WRP_BASE64:
            rv = __wrp_struct_to_base64( msg, &data );
            break;

        case WRP_STRING:
            rv = __wrp_struct_to_string( msg, &data );
            break;

        default:
            return -2;
    }

    if( NULL != bytes ) {
        *bytes = data;
    } else {
        free( data );
    }

    return rv;
}


/* See wrp-c.h for details. */
char* wrp_struct_to_string( const wrp_msg_t *msg )
{
    char *string;

    string = NULL;

    if( NULL != msg ) {
        __wrp_struct_to_string( msg, &string );
    }

    return string;
}

/* See wrp-c.h for details. */
ssize_t wrp_to_struct( const void *bytes, const size_t length,
                       const enum wrp_format fmt, wrp_msg_t **msg )
{
    ssize_t rv;

    if( NULL == bytes || length <= 0 ) {
        return -1;
    }

    switch( fmt ) {
        case WRP_BYTES:
            rv = __wrp_bytes_to_struct( bytes, length, msg );
            break;

        case WRP_BASE64:
            rv = __wrp_base64_to_struct( bytes, length, msg );
            break;

        default:
            return -2;
    }

    return rv;
}

/* See wrp-c.h for details. */
void wrp_free_struct( wrp_msg_t *msg )
{
    switch( mst->msg_type ) {
        case WRP_MSG_TYPE__REQ:
            free( msg->u.req.transaction_uuid );
            free( msg->u.req.source );
            free( msg->u.req.dest );
            free( msg->u.req.payload );

            if( NULL != msg->u.req.headers ) {
                free( msg->u.req.headers );
            }

            break;

        case WRP_MSG_TYPE__EVENT:
            free( msg->u.event.source );
            free( msg->u.event.dest );
            free( msg->u.event.payload );

            if( NULL != msg->u.event.headers ) {
                free( msg->u.event.headers );
            }

            break;

        default:
            break;
    }

    free( msg );
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/**
 *  Do the work of converting into msgpack binary format.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the place to put the output (never NULL)
 *
 *  @return size of buffer (in bytes) on success or less then 1 on error
 */

static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes )
{
    ssize_t rv;
    const struct wrp_req_msg *req = & ( msg->u.req );
    const struct wrp_event_msg *event = & ( msg->u.event );
    const struct money_trace_spans *time_spans = NULL;

    if( NULL == msg ) {
        return -1;
    }

    //convert to binary bytes using msgpack
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            return -1;

        case WRP_MSG_TYPE__REQ:

            //spans allocation is required only when include timing spans is enabled

            if( req->include_spans == false ) {
                time_spans = NULL;
            } else {
                time_spans = & ( req->spans );
            }

            rv = __wrp_pack_structure( msg->msg_type, req->source, req->dest, req->transaction_uuid,
                                       req->include_spans, time_spans, req->payload, req->payload_size, req->headers, bytes );
            break;

        case WRP_MSG_TYPE__EVENT:
            rv = __wrp_pack_structure( msg->msg_type, event->source, event->dest, NULL, false, NULL,
                                       event->payload, event->payload_size, event->headers, bytes );
            break;

        default:
            break;
    }

    return rv;
}


/**
 *  Do the work of converting into base64 format.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the place to put the output (never NULL)
 *
 *  @return size of buffer (in bytes) on success or less then 1 on error
 */
static ssize_t __wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes )
{
    ssize_t rv;
    size_t base64_buf_size, bytes_size;
    char *bytes_data;
    char *base64_data;

    rv = __wrp_struct_to_bytes( msg, &bytes_data );

    if( rv < 1 ) {
        rv = -100;
        goto failed_to_get_bytes;
    }

    bytes_size = ( size_t ) rv;

    base64_buf_size = b64_get_encoded_buffer_size( rv );

    base64_data = ( char* ) malloc( sizeof( char ) * base64_buf_size );

    if( NULL == base64_data ) {
        rv = -101;
        goto failed_to_malloc;
    }

    b64_encode( ( uint8_t* ) bytes_data, bytes_size, ( uint8_t* ) base64_data );
    *bytes = base64_data;

    /* error handling & cleanup */
    if( rv < 1 ) {
failed_to_malloc:
        free( bytes_data );

failed_to_get_bytes:
        ;   /* Nothing left to clean up. */
    }

    return rv;
}


/**
 *  Split out the different types to different functions.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the output of the conversion
 *
 *  @return the number of bytes in the string or less then 1 on error
 */
static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            return __wrp_auth_struct_to_string( &msg->u.auth, bytes );

        case WRP_MSG_TYPE__REQ:
            return __wrp_req_struct_to_string( &msg->u.req, bytes );

        case WRP_MSG_TYPE__EVENT:
            return __wrp_event_struct_to_string( &msg->u.event, bytes );

        default:
            break;
    }

    return -1;
}


/**
 *  Convert the auth structure to a string.
 *
 *  @param auth  [in]  the message to convert
 *  @param bytes [out] the output of the conversion
 *
 *  @return the number of bytes in the string or less then 1 on error
 */
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth,
        char **bytes )
{
    const char const *auth_fmt = "wrp_auth_msg {\n"
                                 "    .status = %d\n"
                                 "}\n";

    char *data;
    size_t length;


    length = snprintf( NULL, 0, auth_fmt, auth->status );

    if( NULL != bytes ) {
        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            sprintf( data, auth_fmt, auth->status );
            data[length] = '\0';
            *bytes = data;
        } else {
            return -1;
        }
    }

    return length;
}


/**
 *  Convert the req structure to a string.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the output of the conversion
 *
 *  @return the number of bytes in the string or less then 1 on error
 */
static ssize_t __wrp_req_struct_to_string( const struct wrp_req_msg *req, char **bytes )
{
    const char const *req_fmt = "wrp_req_msg {\n"
                                "    .transaction_uuid = %s\n"
                                "    .source           = %s\n"
                                "    .dest             = %s\n"
                                "    .headers          = %s\n"
                                "    .include_spans    = %s\n"
                                "    .spans            = %s\n"
                                "    .payload_size     = %zd\n"
                                "}\n";

    size_t length;
    char *headers;
    char *spans;

    headers = __get_header_string( req->headers );
    spans = __get_spans_string( &req->spans );

    length = snprintf( NULL, 0, req_fmt, req->transaction_uuid, req->source,
                       req->dest, headers, ( req->include_spans ? "true" : "false" ),
                       spans, req->payload_size );

    if( NULL != bytes ) {
        char *data;

        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            sprintf( data, req_fmt, req->transaction_uuid, req->source,
                     req->dest, headers, ( req->include_spans ? "true" : "false" ),
                     spans, req->payload_size );
            data[length] = '\0';

            *bytes = data;
        } else {
            length = -1;
        }
    }

    if( __empty_list != headers ) {
        free( headers );
    }

    if( __empty_list != spans ) {
        free( spans );
    }

    return length;
}


/**
 *  Convert the event structure to a string.
 *
 *  @param event [in]  the message to convert
 *  @param bytes [out] the output of the conversion
 *
 *  @return the number of bytes in the string or less then 1 on error
 */
static ssize_t __wrp_event_struct_to_string( const struct wrp_event_msg *event,
        char **bytes )
{
    const char const *event_fmt = "wrp_event_msg {\n"
                                  "    .source           = %s\n"
                                  "    .dest             = %s\n"
                                  "    .headers          = %s\n"
                                  "    .payload_size     = %zd\n"
                                  "}\n";

    size_t length;
    char *headers;

    headers = __get_header_string( event->headers );

    length = snprintf( NULL, 0, event_fmt, event->source, event->dest,
                       headers, event->payload_size );

    if( NULL != bytes ) {
        char *data;

        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            sprintf( data, event_fmt, event->source, event->dest, headers,
                     event->payload_size );
            data[length] = '\0';

            *bytes = data;
        } else {
            length = -1;
        }
    }

    if( __empty_list != headers ) {
        free( headers );
    }

    return length;
}


/**
 *  Converts the list of headers into a string to print.
 *
 *  @note The caller must check to see if the value is equal to __empty_list
 *        and not free it if equal.  If not equal it must be freed.
 *  @note This function never returns NULL.
 *
 *  @param headers [in] the headers to make into a string
 *
 *  @return The string representation of the headers.
 */
static char* __get_header_string( char **headers )
{
    char *rv;

    rv = ( char* ) __empty_list;

    if( headers ) {
        char *tmp;
        size_t i;
        int comma;
        size_t length;

        comma = 0;
        length = 2; /* For ' characters. */

        for( i = 0; NULL != headers[i]; i++ ) {
            length += comma;
            length += strlen( headers[i] );
            comma = 2;
        }

        tmp = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != tmp ) {
            const char *comma;

            rv = tmp;

            comma = "";
            tmp = strcat( tmp, "'" );

            for( i = 0; NULL != headers[i]; i++ ) {
                tmp = strcat( tmp, comma );
                tmp = strcat( tmp, headers[i] );
                comma = ", ";
            }

            tmp = strcat( tmp, "'" );
        }
    }

    return rv;
}

/**
 *  Encode/Pack the wrp message structure using msgpack
 *
 *  @param  [in]  the messages to convert
 *  @param data [out] the encoded output
 *
 *  @return the number of bytes in the string or less then 1 on error*/

static ssize_t __wrp_pack_structure( int msg_type, char *source, char* dest,
                                     char* transaction_uuid, bool includeTimingValues,
                                     const struct money_trace_spans *timeSpan, char* payload, size_t payload_size,
                                     char **headers, char **data )
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    ssize_t rv;
    unsigned int cnt = 0;
    int wrp_map_size = WRP_MAP_SIZE;

    /***   Start of Msgpack Encoding  ***/

    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    // Change wrp_map_size value depending on if optional fields timeSpan and headers are present
    if( transaction_uuid != NULL ) {
        wrp_map_size++;
    }

    if( timeSpan != NULL ) {
        wrp_map_size = wrp_map_size + 2;
    }

    if( headers != NULL ) {
        wrp_map_size++;
    }

    printf( "wrp size is:%d\n", wrp_map_size );
    msgpack_pack_map( &pk, wrp_map_size );

    printf( "msg_type %d\n", msg_type );
    msgpack_pack_str( &pk, strlen( WRP_MSG_TYPE ) );
    msgpack_pack_str_body( &pk, WRP_MSG_TYPE, strlen( WRP_MSG_TYPE ) );
    msgpack_pack_int( &pk, msg_type );

    if( source != NULL ) {
        msgpack_pack_str( &pk, strlen( WRP_SOURCE ) );
        msgpack_pack_str_body( &pk, WRP_SOURCE, strlen( WRP_SOURCE ) );
        msgpack_pack_str( &pk, strlen( source ) );
        msgpack_pack_str_body( &pk, source, strlen( source ) );
    }

    if( dest != NULL ) {
        msgpack_pack_str( &pk, strlen( WRP_DESTINATION ) );
        msgpack_pack_str_body( &pk, WRP_DESTINATION, strlen( WRP_DESTINATION ) );
        msgpack_pack_str( &pk, strlen( dest ) );
        msgpack_pack_str_body( &pk, dest, strlen( dest ) );
    }

    if( transaction_uuid != NULL ) {
        msgpack_pack_str( &pk, strlen( WRP_TRANSACTION_ID ) );
        msgpack_pack_str_body( &pk, WRP_TRANSACTION_ID, strlen( WRP_TRANSACTION_ID ) );
        msgpack_pack_str( &pk, strlen( transaction_uuid ) );
        msgpack_pack_str_body( &pk, transaction_uuid, strlen( transaction_uuid ) );
    }

    if( headers != NULL ) {
        msgpack_pack_str( &pk, strlen( WRP_HEADERS ) );
        msgpack_pack_str_body( &pk, WRP_HEADERS, strlen( WRP_HEADERS ) );
        msgpack_pack_str( &pk, strlen( headers[0] ) );
        msgpack_pack_str_body( &pk, headers, strlen( headers[0] ) );
    }

    //timing values available only for msg req not for msg event
    if( msg_type == WRP_MSG_TYPE__REQ ) {
        if( includeTimingValues ) {
            msgpack_pack_str( &pk, strlen( WRP_INCLUDE_TIMING_VALUES ) );
            msgpack_pack_str_body( &pk, WRP_INCLUDE_TIMING_VALUES,
                                   strlen( WRP_INCLUDE_TIMING_VALUES ) );
            msgpack_pack_true( &pk );

            msgpack_pack_str( &pk, strlen( WRP_TIMING_VALUES ) );
            msgpack_pack_str_body( &pk, WRP_TIMING_VALUES, strlen( WRP_TIMING_VALUES ) );

            if( timeSpan != NULL ) {
                if( timeSpan->spans != NULL ) {
                    msgpack_pack_array( &pk, timeSpan->count );

                    for( cnt = 0; cnt < timeSpan->count; cnt++ ) {
                        msgpack_pack_array( &pk, 3 );
                        msgpack_pack_str( &pk, strlen( timeSpan->spans[cnt].name ) );
                        msgpack_pack_str_body( &pk, timeSpan->spans[cnt].name,
                                               strlen( timeSpan->spans[cnt].name ) );

                        msgpack_pack_uint64( &pk, timeSpan->spans[cnt].start );

                        msgpack_pack_uint32( &pk, timeSpan->spans[cnt].duration );
                    }
                }
            }
        }
    }

    if( payload != NULL ) {
        msgpack_pack_str( &pk, strlen( WRP_PAYLOAD ) );
        msgpack_pack_str_body( &pk, WRP_PAYLOAD, strlen( WRP_PAYLOAD ) );
        msgpack_pack_bin( &pk, payload_size );
        msgpack_pack_bin_body( &pk, payload, payload_size );
    }

    if( data != NULL && sbuf.data != NULL ) {
        *data = ( char * ) malloc( sizeof( char ) * ( sbuf.size + 1 ) );
        strncpy( *data, sbuf.data, sbuf.size + 1 );
        rv = sbuf.size;
    }

    msgpack_sbuffer_destroy( &sbuf );

    /***   End of Msgpack Encoding   ***/
    if( data == NULL ) {
        return -1;    //failure in packing
    }

    return rv;
}

/**
 *  Converts the list of times into a string to print.
 *
 *  @note The caller must check to see if the value is equal to __empty_list
 *        and not free it if equal.  If not equal it must be freed.
 *  @note This function never returns NULL.
 *
 *  @param spans [in] the spans to make into a string
 *
 *  @return The string representation of the times.
 */
static char* __get_spans_string( const struct money_trace_spans *spans )
{
    char *rv;
    size_t count;

    count = spans->count;
    rv = ( char* ) __empty_list;

    if( 0 < count ) {
        char *buffer;
        size_t length, i;
        const struct money_trace_span *p;

        length = 0;
        p = spans->spans;

        for( i = 0; i < count; i++, p++ ) {
            length += snprintf( NULL, 0, "\n        %s: %" PRIu64 " - %" PRIu32,
                                p->name, p->start, p->duration );
        }

        buffer = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != buffer ) {
            rv = buffer;

            p = spans->spans;

            for( i = 0; i < count; i++, p++ ) {
                buffer += sprintf( buffer, "\n        %s: %" PRIu64 " - %" PRIu32,
                                   p->name, p->start, p->duration );
            }

            *buffer = '\0';
        }
    }

    return rv;
}

/**
 * @brief decodeRequest function to unpack the request received from server.
 *
 * @param[in] deserialized msgpack object to deserialize
 * @param[in] msg_type type of the message
 * @param[in] source_ptr source from which the response is generated
 * @param[in] dest_ptr Destination to which it has to be sent
 * @param[in] transaction_uuid_ptr Id
 * @param[in] statusValue status value of authorization request
 * @param[in] payload_ptr bin payload to be extracted from request
 * @param[in] includeTimingValues to include timing values
 */
static void decodeRequest( msgpack_object deserialized, int *msgType, char** source_ptr,
                           char** dest_ptr, char** transaction_uuid_ptr, char ***headers_ptr, int *statusValue,
                           char** payload_ptr, bool *includeTimingValues )
{
    unsigned int i = 0;
    int keySize = 0;
    char* keyString = NULL;
    char* keyName = NULL;
    int sLen = 0;
    int binValueSize = 0, StringValueSize = 0;
    char* NewStringVal, *StringValue;
    char* keyValue = NULL;

    char *transaction_uuid = NULL;
    char **headers = NULL;
    char *source = NULL;
    char *dest = NULL;
    char *payload = NULL;

    *source_ptr = NULL;
    *dest_ptr = NULL;
    *payload_ptr = NULL;

    msgpack_object_kv* p = deserialized.via.map.ptr;

    while( i < deserialized.via.map.size ) {
        sLen = 0;
        msgpack_object keyType = p->key;
        msgpack_object ValueType = p->val;
        keySize = keyType.via.str.size;
        keyString = ( char* ) malloc( keySize + 1 );
        keyName = NULL;
        keyName = getKey_MsgtypeStr( keyType, keySize, keyString );

        if( keyName != NULL ) {
            switch( ValueType.type ) {
                case MSGPACK_OBJECT_POSITIVE_INTEGER: {
                    if( strcmp( keyName, WRP_MSG_TYPE ) == 0 ) {
                        *msgType = ValueType.via.i64;
                    } else if( strcmp( keyName, WRP_STATUS ) == 0 ) {
                        *statusValue = ValueType.via.i64;
                    }
                }
                break;

                case MSGPACK_OBJECT_BOOLEAN: {
                    if( strcmp( keyName, WRP_INCLUDE_TIMING_VALUES ) == 0 ) {
                        *includeTimingValues = ValueType.via.boolean ? true : false;
                    }
                }
                break;

                case MSGPACK_OBJECT_STR: {

                    StringValueSize = ValueType.via.str.size;
                    NewStringVal = ( char* ) malloc( StringValueSize + 1 );
                    StringValue = getKey_MsgtypeStr( ValueType, StringValueSize, NewStringVal );

                    if( strcmp( keyName, WRP_SOURCE ) == 0 ) {
                        sLen = strlen( StringValue );
                        source = ( char * ) malloc( sLen + 1 );
                        strncpy( source, StringValue, sLen );
                        source[sLen] = '\0';
                        *source_ptr = source;
                    }


                    else if( strcmp( keyName, WRP_DESTINATION ) == 0 ) {
                        sLen = strlen( StringValue );
                        dest = ( char * ) malloc( sLen + 1 );
                        strncpy( dest, StringValue, sLen );
                        dest[sLen] = '\0';
                        *dest_ptr = dest;
                    }

                    else if( strcmp( keyName, WRP_TRANSACTION_ID ) == 0 ) {
                        sLen = strlen( StringValue );
                        transaction_uuid = ( char * ) malloc( sLen + 1 );
                        strncpy( transaction_uuid, StringValue, sLen );
                        transaction_uuid[sLen] = '\0';
                        *transaction_uuid_ptr = transaction_uuid;
                    }

                    else if( strcmp( keyName, WRP_HEADERS ) == 0 ) {
                        sLen = strlen( StringValue );
                        *headers = ( char * ) malloc( sLen + 1 );
                        strncpy( *headers, StringValue, sLen );
                        *headers[sLen] = '\0';
                        **headers_ptr = *headers;
                    }

                    WRP_FREE( NewStringVal );
                }
                break;

                case MSGPACK_OBJECT_BIN: {
                    if( strcmp( keyName, WRP_PAYLOAD ) == 0 ) {
                        binValueSize = ValueType.via.bin.size;
                        payload = ( char* ) malloc( binValueSize + 1 );
                        keyValue = NULL;
                        keyValue = getKey_MsgtypeBin( ValueType, binValueSize, payload );

                        if( keyValue != NULL ) {
                            printf( "Binary payload %s\n", keyValue );
                        }

                        *payload_ptr = keyValue;
                    }
                }
                break;

                default:
                    printf( "Unknown Data Type" );
            }
        }

        p++;
        i++;

        WRP_FREE( keyString );
    }
}

/*
* @brief Returns the value of a given key.
* @param[in] key key name with message type string.
*/
static char* getKey_MsgtypeStr( const msgpack_object key, const size_t keySize,
                                char* keyString )
{
    const char* keyName = key.via.str.ptr;
    strncpy( keyString, keyName, keySize );
    keyString[keySize] = '\0';
    return keyString;
}


/*
 * @brief Returns the value of a given key.
 * @param[in] key key name with message type binary.
 */
static char* getKey_MsgtypeBin( const msgpack_object key, const size_t binSize,
                                char* keyBin )
{
    const char* keyName = key.via.bin.ptr;
    memcpy( keyBin, keyName, binSize );
    keyBin[binSize] = '\0';
    return keyBin;
}

/*
**
 *  Decode msgpack encoded data in bytes and convert to wrp struct
 *
 *  @param bytes [in]  msgpack encoded message in bytes
 *  @param length [in] length of encoded message
 *  @param msg_ptr [inout] wrp_msg_t struct initialized using bytes
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */

static ssize_t __wrp_bytes_to_struct( const void *bytes, const size_t length,
                                      wrp_msg_t **msg_ptr )
{
    msgpack_zone mempool;
    msgpack_object deserialized;
    msgpack_unpack_return unpack_ret;

    int msgType = -1;
    int statusValue = -1;

    char* source = NULL;
    char* dest = NULL;
    char* transaction_uuid = NULL;
    char** headers = NULL;
    char *payload = NULL;
    bool includeTimingValues = false;

    wrp_msg_t *msg = NULL;


    if( bytes != NULL ) {
        printf( "unpacking encoded data\n" );

        msgpack_zone_init( &mempool, 2048 );
        unpack_ret = msgpack_unpack( bytes, length, NULL, &mempool, &deserialized );
        printf( "unpack_ret:%d\n", unpack_ret );

        switch( unpack_ret ) {
            case MSGPACK_UNPACK_SUCCESS:

                msgpack_object_print( stdout, deserialized );

                if( deserialized.via.map.size != 0 ) {
                    decodeRequest( deserialized, &msgType, &source, &dest, &transaction_uuid, &headers,
                                   &statusValue, &payload, &includeTimingValues );
                }

                msgpack_zone_destroy( &mempool );
                msg = ( wrp_msg_t * ) malloc( sizeof( wrp_msg_t ) );

                switch( msgType ) {
                    case WRP_MSG_TYPE__AUTH:

                        msg->msg_type = msgType;
                        msg->u.auth.status = statusValue;
                        return length;

                    case WRP_MSG_TYPE__REQ:

                        msg->msg_type = msgType;
                        msg->u.req.source = source;
                        msg->u.req.dest = dest;
                        msg->u.req.transaction_uuid = transaction_uuid;
                        msg->u.req.headers = headers;
                        msg->u.req.include_spans = includeTimingValues;
                        msg->u.req.payload = payload;

                        *msg_ptr = msg;
                        return length;

                    case WRP_MSG_TYPE__EVENT:

                        msg->msg_type = msgType;
                        msg->u.event.source = source;
                        msg->u.event.dest = dest;
                        msg->u.event.payload = payload;

                        *msg_ptr = msg;
                        return length;

                    default:
                        return -1;
                }

            case MSGPACK_UNPACK_EXTRA_BYTES: {
                printf( "MSGPACK_UNPACK_EXTRA_BYTES\n" );
                return -1;
            }

            case MSGPACK_UNPACK_CONTINUE: {
                printf( "MSGPACK_UNPACK_CONTINUE\n" );
                return -1;
            }

            case MSGPACK_UNPACK_PARSE_ERROR: {
                printf( "MSGPACK_UNPACK_PARSE_ERROR\n" );
                return -1;
            }

            case MSGPACK_UNPACK_NOMEM_ERROR: {
                printf( "MSGPACK_UNPACK_NOMEM_ERROR\n" );
                return -1;
            }

            default:
                return -1;
        }

    }

    printf( "bytes is NULL\n" );
    return -1;

}

/*
**
 *  Decode base64 encoded msgpack encoded data in bytes and convert to wrp struct
 *
 *  @param bytes [in]  base64 encoded msgpack encoded message in bytes
 *  @param length [in] length of base64 encoded message
 *  @param msg_ptr [inout] wrp_msg_t struct initialized
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */

static ssize_t __wrp_base64_to_struct( const void *base64_data, const size_t base64_size,
                                       wrp_msg_t **msg_ptr )
{
    ssize_t rv;
    size_t length, decodeMsgSize;
    char *bytes;

    decodeMsgSize = b64_get_decoded_buffer_size( base64_size );

    bytes = ( char * ) malloc( sizeof( char ) * decodeMsgSize );

    length = b64_decode( ( uint8_t * ) base64_data, base64_size, ( uint8_t * ) bytes );

    rv = __wrp_bytes_to_struct( bytes, length, msg_ptr );

    free( bytes );

    return rv;
}


