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

#include <trower-base64/base64.h>

#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static const char const *__empty_list = "''";
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth, char **bytes );
static ssize_t __wrp_req_struct_to_string( const struct wrp_req_msg *req, char **bytes );
static ssize_t __wrp_event_struct_to_string( const struct wrp_event_msg *event, char **bytes );
static char* __get_header_string( char **headers );
static char* __get_spans_string( const struct money_trace_spans *spans );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/* See wrp-c.h for details. */
ssize_t wrp_struct_to( const wrp_msg_t *msg, const enum wrp_format fmt,
                       void **bytes )
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


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes )
{
    (void) msg;
    (void) bytes;

    return -1;
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
    bytes_size = (size_t) rv;

    base64_buf_size = b64_get_encoded_buffer_size( rv );

    base64_data = (char*) malloc( sizeof(char) * base64_buf_size );
    if( NULL == base64_data ) {
        rv = -101;
        goto failed_to_malloc;
    }

    b64_encode( (uint8_t*) bytes_data, bytes_size, (uint8_t*) base64_data );
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
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth, char **bytes )
{
    const char const *auth_fmt = "wrp_auth_msg {\n"
                                 "    .status = %d\n"
                                 "}\n";

    char *data;
    size_t length;


    length = snprintf( NULL, 0, auth_fmt, auth->status );

    if( NULL != bytes ) {
        data = (char*) malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */
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
                                "    .spans            = %s\n"
                                "    .payload_size     = %zd\n"
                                "}\n";

    size_t length;
    char *headers;
    char *spans;


    headers = __get_header_string( req->headers );
    spans = __get_spans_string( &req->spans );

    length = snprintf( NULL, 0, req_fmt, req->transaction_uuid, req->source,
                       req->dest, headers, spans, req->payload_size );

    if( NULL != bytes ) {
        char *data;

        data = (char*) malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */
        if( NULL != data ) {
            sprintf( data, req_fmt, req->transaction_uuid, req->source,
                     req->dest, headers, spans, req->payload_size );
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
    (void) event;
    (void) bytes;

    return -1;
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

    rv = (char*) __empty_list;
    if( headers ) {
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

        rv = (char*) malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */
        if( NULL != rv ) {
            char *tmp;
            const char *comma;

            comma = "";
            tmp = rv;
            tmp = strcat( tmp, "'" );
            for( i = 0; NULL != headers[i]; i++ ) {
                tmp = strcat( tmp, comma );
                tmp = strcat( tmp, headers[i] );
                comma = ", ";
            }
            tmp = strcat( tmp, "'" );
        } else {
            rv = (char*) __empty_list;
        }
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
    rv = (char*) __empty_list;

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
    
        buffer = (char*) malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */
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
