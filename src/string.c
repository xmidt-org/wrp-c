/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "string.h"
#include "utils.h"
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
static const char *__empty_list = "''";

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_keep_alive_to_string( char **bytes );
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth,
        char **bytes );
static ssize_t __wrp_req_struct_to_string( const struct wrp_req_msg *req, char **bytes );
static ssize_t __wrp_event_struct_to_string( const struct wrp_event_msg *event,
        char **bytes );
static char* __get_header_string( headers_t *headers );
static char* __get_spans_string( const struct money_trace_spans *spans );
static char* __get_partner_ids_string( partners_t *partner_ids );
static void __special_free( char *s );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

ssize_t internal_struct_to_string( const wrp_msg_t *msg, char **bytes )
{
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__SVC_ALIVE:
            return __wrp_keep_alive_to_string( bytes );

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


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_keep_alive_to_string( char **bytes )
{
    const char *keep_alive_fmt = "wrp_keep_alive_msg {\n"
                                 "}\n";
    size_t len;

    *bytes = mlaprintf( &len, keep_alive_fmt );
    if( NULL == *bytes ) {
        len = -1;
    }

    return len;
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
    const char *auth_fmt = "wrp_auth_msg {\n"
                           "    .status = %d\n"
                           "}\n";
    size_t len;

    *bytes = mlaprintf( &len, auth_fmt, auth->status );
    if( NULL == *bytes ) {
        len = -1;
    }

    return len;
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
    const char *req_fmt = "wrp_req_msg {\n"
                          "    .transaction_uuid = %s\n"
                          "    .source           = %s\n"
                          "    .dest             = %s\n"
                          "    .partner_ids      = %s\n"
                          "    .headers          = %s\n"
                          "    .content_type     = %s\n"
                          "    .accept           = %s\n"
                          "    .include_spans    = %s\n"
                          "    .spans            = %s\n"
                          "    .payload_size     = %zd\n"
                          "}\n";
    size_t len;
    char *headers;
    char *spans;
    char *partner_ids;

    headers = __get_header_string( req->headers );
    spans = __get_spans_string( &req->spans );
    partner_ids = __get_partner_ids_string( req->partner_ids );

    *bytes = mlaprintf( &len, req_fmt, req->transaction_uuid, req->source,
                        req->dest, partner_ids, headers, req->content_type,
                        req->accept, ( req->include_spans ? "true" : "false" ),
                        spans, req->payload_size );
    if( NULL == *bytes ) {
        len = -1;
    }

    __special_free( headers );
    __special_free( spans );
    __special_free( partner_ids );

    return len;
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
    const char *event_fmt = "wrp_event_msg {\n"
                            "    .source           = %s\n"
                            "    .dest             = %s\n"
                            "    .partner_ids      = %s\n"
                            "    .headers          = %s\n"
                            "    .content_type     = %s\n"
                            "    .payload_size     = %zd\n"
                            "}\n";
    size_t len;
    char *headers;
    char *partner_ids;

    headers = __get_header_string( event->headers );
    partner_ids = __get_partner_ids_string( event->partner_ids );

    *bytes = mlaprintf( &len, event_fmt, event->source, event->dest, partner_ids,
                        headers, event->content_type, event->payload_size );
    if( NULL == *bytes ) {
        len = -1;
    }

    __special_free( headers );
    __special_free( partner_ids );

    return len;
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
static char* __get_header_string( headers_t *headers )
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

        for( i = 0; i < headers->count; i++ ) {
            length += comma;
            length += strlen( headers->headers[i] );
            comma = 2;
        }

        tmp = malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */

        if( NULL != tmp ) {
            const char *comma;
            rv = tmp;
            comma = "";
            *tmp = '\0';
            tmp = strcat( tmp, "'" );

            for( i = 0; i < headers->count; i++ ) {
                tmp = strcat( tmp, comma );
                tmp = strcat( tmp, headers->headers[i] );
                comma = ", ";
            }

            tmp = strcat( tmp, "'" );
        }
    }

    return rv;
}


static char* __get_partner_ids_string( partners_t *partner_ids )
{
    char *rv;
    rv = ( char* ) __empty_list;

    if( partner_ids ) {
        char *tmp;
        size_t i;
        int comma;
        size_t length;
        comma = 0;
        length = 2; /* For ' characters. */

        for( i = 0; i < partner_ids->count; i++ ) {
            length += comma;
            length += strlen( partner_ids->partner_ids[i] );
            comma = 2;
        }

        tmp = malloc( sizeof(char) * (length + 1) );   /* +1 for '\0' */

        if( NULL != tmp ) {
            const char *comma;
            rv = tmp;
            comma = "";
            *tmp = '\0';
            tmp = strcat( tmp, "'" );

            for( i = 0; i < partner_ids->count; i++ ) {
                tmp = strcat( tmp, comma );
                tmp = strcat( tmp, partner_ids->partner_ids[i] );
                comma = ", ";
            }

            tmp = strcat( tmp, "'" );
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

        buffer = malloc( sizeof(char) * ( length + 1 ) );   /* +1 for '\0' */

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


static void __special_free( char *s )
{
    if( __empty_list != s ) {
        free( s );
    }
}
