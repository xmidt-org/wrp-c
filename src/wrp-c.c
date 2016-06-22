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
#include <trower-base64/base64.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
static const char const *empty_list = "''";
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_auth_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_req_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_event_struct_to_string( const wrp_msg_t *msg, char **bytes );
static size_t __get_header_string( const char **headers );
static char* __get_timing_string( const struct wrp_timing_value *timing_values );

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


static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            return __wrp_auth_struct_to_string( msg, bytes );
        case WRP_MSG_TYPE__REQ:
            return __wrp_req_struct_to_string( msg, bytes );
        case WRP_MSG_TYPE__EVENT:
            return __wrp_event_struct_to_string( msg, bytes );
        default:
            break;
    }
    
    return -1;
}

static ssize_t __wrp_auth_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    const char const *auth_fmt = "wrp_auth_msg {\n"
                                 "    .status = %d\n"
                                 "}\n";

    const struct wrp_auth_msg *auth;
    char *data;
    size_t length;


    auth = &msg->u.auth;

    length = snprintf( NULL, 0, auth_fmt, auth->status );
    length++;   /* For trailing '\0' */

    data = (char*) malloc( sizeof(char) * length );
    if( NULL != *bytes ) {
        snprintf( data, length, auth_fmt, auth->status );
    }
    data[length] = '\0';

    return length;
}


static ssize_t __wrp_req_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    const char const *req_fmt = "wrp_req_msg {\n"
                                "    .transaction_uuid = %s\n"
                                "    .source           = %s\n"
                                "    .dest             = %s\n"
                                "    .headers          = %s\n"
                                "    .timing_values    = %s\n"
                                "    .payload_size     = %zd\n"
                                "}\n";

    const struct wrp_req_msg *req;
    char *data;
    size_t length;


    req = &msg->u.req;

    length = snprintf( NULL, 0, req_fmt, req->transaction_uuid,
                       req->source, req->dest, "", "", req->payload_size );
    length++;   /* For trailing '\0' */

    data = (char*) malloc( sizeof(char) * length );
    if( NULL != *bytes ) {
        snprintf( data, length, req_fmt, req->transaction_uuid,
                  req->source, req->dest, "", "", req->payload_size );
    }
    data[length] = '\0';

    return length;

}

static ssize_t __wrp_event_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    return -1;
}


static size_t __get_header_string( const char **headers )
{
    if( headers ) {
        const char *p;
        int comma = 0;

        rv = 0;
        for( p = *headers; NULL != p; p++ ) {
            rv += comma;
            rv += strlen( p );
            comma = 2;
        }

        return NULL;
    }

    return (char*) empty_list;
}


static char* __get_timing_string( const struct wrp_timing_value *timing_values )
{
    if( timing_values ) {
        size_t length;
        char *rv;
        const struct wrp_timing_value *p;

        length = 0;
        for( p = timing_values; NULL != p; p++ ) {
            length += snprintf( NULL, 0, "        %s: %ld.%.06ld - %ld.%.06ld\n",
                                p->name, p->start.tv_sec, p->start.tv_usec,
                                p->end.tv_sec, p->end.tv_usec );
        }
        length++; /* For the trailing '\0' */
    
        rv = (char*) malloc( sizeof(char) * length );
        if( NULL != rv ) {
            char *tmp;

            tmp = rv;
            for( p = timing_values; NULL != p; p++ ) {
                tmp += snprintf( tmp, "        %s: %ld.%.06ld - %ld.%.06ld\n",
                                p->name, p->start.tv_sec, p->start.tv_usec,
                                p->end.tv_sec, p->end.tv_usec );
            }
            *tmp = '\0';
        }
    }

    return (char*) empty_list;
}
