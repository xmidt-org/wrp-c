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
/* none */

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
    size_t len;

    *bytes = mlaprintf( &len, "wrp_auth_msg {\n"
                              "    .status = %d\n"
                              "}\n",
                        auth->status );

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
    size_t len;
    char *headers     = __get_header_string( req->headers );
    char *spans       = __get_spans_string( &req->spans );
    char *partner_ids = __get_partner_ids_string( req->partner_ids );

    *bytes = mlaprintf( &len, "wrp_req_msg {\n"
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
                              "}\n",
                        req->transaction_uuid,
                        req->source,
                        req->dest,
                        (partner_ids ? partner_ids : "''"),
                        (headers ? headers : "''"),
                        req->content_type,
                        req->accept,
                        (req->include_spans ? "true" : "false"),
                        (spans ? spans : "''"),
                        req->payload_size );

    if( NULL == *bytes ) {
        len = -1;
    }

    if( headers )     free( headers );
    if( spans )       free( spans );
    if( partner_ids ) free( partner_ids );

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
    size_t len;
    char *headers     = __get_header_string( event->headers );
    char *partner_ids = __get_partner_ids_string( event->partner_ids );

    *bytes = mlaprintf( &len, "wrp_event_msg {\n"
                              "    .source           = %s\n"
                              "    .dest             = %s\n"
                              "    .partner_ids      = %s\n"
                              "    .headers          = %s\n"
                              "    .content_type     = %s\n"
                              "    .payload_size     = %zd\n"
                              "}\n",
                        event->source,
                        event->dest,
                        (partner_ids ? partner_ids : "''"),
                        (headers ? headers : "''"),
                        event->content_type,
                        event->payload_size );

    if( NULL == *bytes ) {
        len = -1;
    }

    if( headers )     free( headers );
    if( partner_ids ) free( partner_ids );

    return len;
}


/**
 *  Converts the list of headers into a string to print.
 *
 *  @param headers [in] the headers to make into a string
 *
 *  @return The string representation of the headers or NULL if none.
 */
static char* __get_header_string( headers_t *headers )
{
    char *rv = NULL;
    char *p = NULL;
    size_t len = 0;
    const char *comma = "";

    if( !headers || (0 == headers->count) ) {
        return NULL;
    }

    for( size_t i = 0; i < headers->count; i++ ) {
        len += strlen( headers->headers[i] );
    }
    len += 2;                           /* For '' characters. */
    len += 2 * (headers->count - 1);    /* For ", " between headers */
    len += 1;                           /* For trailing '\0' */

    rv = calloc( len, sizeof(char) );
    if( !rv ) {
        return NULL;
    }

    p = wrp_append( rv, "'" );
    for( size_t i = 0; i < headers->count; i++ ) {
        p = wrp_append( p, comma );
        p = wrp_append( p, headers->headers[i] );
        comma = ", ";
    }
    wrp_append( p, "'" );

    return rv;
}


static char* __get_partner_ids_string( partners_t *partner_ids )
{
    char *rv = NULL;
    char *p = NULL;
    size_t len = 0;
    const char *comma = "";

    if( !partner_ids || (0 == partner_ids->count) ) {
        return NULL;
    }

    for( size_t i = 0; i < partner_ids->count; i++ ) {
        len += strlen( partner_ids->partner_ids[i] );
    }
    len += 2;                               /* For '' characters. */
    len += 2 * (partner_ids->count - 1);    /* For ", " between partner_ids */
    len += 1;                               /* For trailing '\0' */

    rv = calloc( len, sizeof(char) );
    if( !rv ) {
        return NULL;
    }

    p = wrp_append( rv, "'" );
    for( size_t i = 0; i < partner_ids->count; i++ ) {
        p = wrp_append( p, comma );
        p = wrp_append( p, partner_ids->partner_ids[i] );
        comma = ", ";
    }
    wrp_append( p, "'" );

    return rv;
}


/**
 *  Converts the list of times into a string to print.
 *
 *  @param spans [in] the spans to make into a string
 *
 *  @return The string representation of the times or NULL if none.
 */
static char* __get_spans_string( const struct money_trace_spans *spans )
{
    static const char *fmt = "\n        %s: %" PRIu64 " - %" PRIu32;
    char *rv = NULL;
    char *p = NULL;
    size_t len = 0;

    if( !spans || (0 == spans->count) ) {
        return NULL;
    }

    for( size_t i = 0; i < spans->count; i++ ) {
        const struct money_trace_span *span = &spans->spans[i];
        len += snprintf( NULL, 0, fmt, span->name, span->start, span->duration );
    }

    len += 1;   /* +1 for '\0' */
    rv = calloc( len, sizeof(char) );
    if( !rv ) {
        return NULL;
    }

    p = rv;

    for( size_t i = 0; i < spans->count; i++ ) {
        const struct money_trace_span *span = &spans->spans[i];
        int used;
        used = snprintf( p, len, fmt, span->name, span->start, span->duration );
        if( used < 1 ) {
            free( rv );
            return NULL;
        }
        p += used;
        len -= used;
    }
    *p = '\0';

    return rv;
}
