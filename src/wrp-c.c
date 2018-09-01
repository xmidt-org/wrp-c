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
#define _GNU_SOURCE 1
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <trower-base64/base64.h>
#include <cimplog/cimplog.h>

#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define LOGGING_MODULE   "WRP-C"

#define WRP_ERROR( ... ) cimplog_error(LOGGING_MODULE, __VA_ARGS__)
#define WRP_INFO( ... )  cimplog_info(LOGGING_MODULE, __VA_ARGS__)
#define WRP_DEBUG( ... ) cimplog_debug(LOGGING_MODULE, __VA_ARGS__)

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
struct wrp_token {
    const char *name;
    size_t length;
};


struct req_res_t {
    int msgType ;
    int statusValue;
    char* source ;
    char* dest ;
    char* transaction_uuid;
    partners_t *partner_ids ;
    headers_t *headers ;
    void *payload ;
    size_t payload_size;
    bool include_spans;
    struct money_trace_spans spans;
    data_t *metadata;
    int status;
    int rdr;
    char *path;
    char *service_name;
    char *url;
    char *content_type;
    char *accept;
};


/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static const char *__empty_list = "''";
#define METADATA_MAP_SIZE                          1
/* Reminder: sizeof("string") includes the trailing '\0', which we don't want. */
static const struct wrp_token WRP_MSG_TYPE      = { .name = "msg_type", .length = sizeof( "msg_type" ) - 1 };
static const struct wrp_token WRP_STATUS        = { .name = "status", .length = sizeof( "status" ) - 1 };
static const struct wrp_token WRP_SOURCE        = { .name = "source", .length = sizeof( "source" ) - 1 };
static const struct wrp_token WRP_DEST          = { .name = "dest", .length = sizeof( "dest" ) - 1 };
static const struct wrp_token WRP_TRANS_ID      = { .name = "transaction_uuid", .length = sizeof( "transaction_uuid" ) - 1 };
static const struct wrp_token WRP_CONTENT_TYPE  = { .name = "content_type", .length = sizeof( "content_type" ) - 1 };
static const struct wrp_token WRP_ACCEPT        = { .name = "accept", .length = sizeof( "accept" ) - 1 };
static const struct wrp_token WRP_HEADERS       = { .name = "headers", .length = sizeof( "headers" ) - 1 };
static const struct wrp_token WRP_PAYLOAD       = { .name = "payload", .length = sizeof( "payload" ) - 1 };
static const struct wrp_token WRP_SPANS         = { .name = "spans", .length = sizeof( "spans" ) - 1 };
static const struct wrp_token WRP_INCLUDE_SPANS = { .name = "include_spans", .length = sizeof( "include_spans" ) - 1 };
static const struct wrp_token WRP_SERVICE_NAME  = { .name = "service_name", .length = sizeof( "service_name" ) - 1 };
static const struct wrp_token WRP_URL           = { .name = "url", .length = sizeof( "url" ) - 1 };
static const struct wrp_token WRP_METADATA      = { .name = "metadata", .length = sizeof( "metadata" ) - 1 };
static const struct wrp_token WRP_RDR           = { .name = "rdr", .length = sizeof( "rdr" ) - 1 };
static const struct wrp_token WRP_PATH          = { .name = "path", .length = sizeof( "path" ) - 1 };
static const struct wrp_token WRP_PARTNER_IDS   = { .name = "partner_ids", .length = sizeof( "partner_ids" ) - 1 };
static const int WRP_MAP_SIZE                   = 4; // mandatory msg_type,source,dest,payload


/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static ssize_t __wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_struct_to_string( const wrp_msg_t *msg, char **bytes );
static ssize_t __wrp_keep_alive_to_string (char **bytes );
static ssize_t __wrp_auth_struct_to_string( const struct wrp_auth_msg *auth,
        char **bytes );
static ssize_t __wrp_req_struct_to_string( const struct wrp_req_msg *req, char **bytes );
static ssize_t __wrp_event_struct_to_string( const struct wrp_event_msg *event,
        char **bytes );
static char* __get_header_string( headers_t *headers );
static char* __get_spans_string( const struct money_trace_spans *spans );
static char* __get_partner_ids_string( partners_t *partner_ids );
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct wrp_token *token,
                                       const char *val );
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );

static ssize_t __wrp_pack_structure( struct req_res_t *encodeReq , char **data );


static ssize_t __wrp_base64_to_struct( const void *base64_data, const size_t base64_size,
                                       wrp_msg_t **msg_ptr );
static ssize_t __wrp_bytes_to_struct( const void *bytes, const size_t length,
                                      wrp_msg_t **msg_ptr );

static void decodeRequest( msgpack_object deserialized, struct req_res_t **decodeReq );
static char* getKey_MsgtypeStr( const msgpack_object key, const size_t keySize,
                                char* keyString );
static char* getKey_MsgtypeBin( const msgpack_object key, const size_t binSize,
                                char* keyBin );
static void __msgpack_maps( msgpack_packer *pk, const data_t *dataMap );
static void decodeMapRequest( msgpack_object deserialized, struct req_res_t **decodeMapReq );
static void mapCommonString( msgpack_packer *pk, struct req_res_t *encodeComReq );
static int alterMap( char * buf );
static char* strdupptr( const char *s, const char *e );


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/* See wrp-c.h for details. */
ssize_t wrp_struct_to( const wrp_msg_t *msg, const enum wrp_format fmt, void **bytes )
{
    char *data;
    ssize_t rv;

    if( NULL == msg || NULL == bytes ) {
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

    *bytes = data;

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

    if( NULL == bytes || length <= 0 || NULL == msg ) {
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
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__REQ:
            free( msg->u.req.transaction_uuid );
            free( msg->u.req.source );
            free( msg->u.req.dest );
            free( msg->u.req.payload );
            if(NULL != msg->u.req.content_type)
            {
                free(msg->u.req.content_type);
            }
            if(NULL != msg->u.req.accept)
            {
                free(msg->u.req.accept);
            }
            if( NULL != msg->u.req.headers ) {
                size_t cnt;

                for( cnt = 0; cnt < msg->u.req.headers->count; cnt++ ) {
                    free( msg->u.req.headers->headers[cnt] );
                }

                free( msg->u.req.headers );
            }
            if( NULL != msg->u.req.metadata ) {
                size_t n = 0;

                while( n < msg->u.req.metadata->count ) {
                    free( msg->u.req.metadata->data_items[n].name );
                    free( msg->u.req.metadata->data_items[n].value );
                    n++;
                }

                free( msg->u.req.metadata->data_items );
                free( msg->u.req.metadata );
            }
            if( NULL != msg->u.req.partner_ids ) {
                size_t i;

                for( i = 0; i < msg->u.req.partner_ids->count; i++ ) {
                    free( msg->u.req.partner_ids->partner_ids[i] );
                }

                free( msg->u.req.partner_ids );
            }
            break;
        case WRP_MSG_TYPE__EVENT:
            free( msg->u.event.source );
            free( msg->u.event.dest );
            free( msg->u.event.payload );
            if(NULL != msg->u.event.content_type)
            {
                free(msg->u.event.content_type);
            }
            if( NULL != msg->u.event.headers ) {
                size_t cnt;

                for( cnt = 0; cnt < ( msg->u.event.headers->count ); cnt++ ) {
                    free( msg->u.event.headers->headers[cnt] );
                }

                free( msg->u.event.headers );
            }
            if( NULL != msg->u.event.metadata ) {
                size_t n = 0;

                while( n < msg->u.event.metadata->count ) {
                    free( msg->u.event.metadata->data_items[n].name );
                    free( msg->u.event.metadata->data_items[n].value );
                    n++;
                }

                free( msg->u.event.metadata->data_items );
                free( msg->u.event.metadata );
            }
            if( NULL != msg->u.event.partner_ids ) {
                size_t i;

                for( i = 0; i < ( msg->u.event.partner_ids->count ); i++ ) {
                    free( msg->u.event.partner_ids->partner_ids[i] );
                }

                free( msg->u.event.partner_ids );
            }
            break;
        case WRP_MSG_TYPE__SVC_REGISTRATION:
            WRP_DEBUG("Free for REGISTRATION \n" );
            free( msg->u.reg.service_name );
            free( msg->u.reg.url );
            break;
        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            WRP_DEBUG("Free for CRUD \n" );

            if( msg->u.crud.source ) {
                free( msg->u.crud.source );
            }

            if( msg->u.crud.dest ) {
                free( msg->u.crud.dest );
            }

            if( msg->u.crud.transaction_uuid != NULL ) {
                free( msg->u.crud.transaction_uuid );
            }

            if( msg->u.crud.path != NULL ) {
                free( msg->u.crud.path );
            }

            if( NULL != msg->u.crud.partner_ids ) {
                size_t i;

                for( i = 0; i < ( msg->u.crud.partner_ids->count ); i++ ) {
                    free( msg->u.crud.partner_ids->partner_ids[i] );
                }

                free( msg->u.crud.partner_ids );
            }

            if( NULL != msg->u.crud.headers ) {
                size_t cnt;

                for( cnt = 0; cnt < ( msg->u.crud.headers->count ); cnt++ ) {
                    free( msg->u.crud.headers->headers[cnt] );
                }

                free( msg->u.crud.headers );
            }

            if(NULL != msg->u.crud.content_type)
            {
                free(msg->u.crud.content_type);
            }
            if(NULL != msg->u.crud.accept)
            {
                free(msg->u.crud.accept);
            }
            if( NULL != msg->u.crud.payload ) {
                free( msg->u.crud.payload );
            }
            if( NULL != msg->u.crud.metadata ) {
                size_t n = 0;

                while( n < msg->u.crud.metadata->count ) {
                    free( msg->u.crud.metadata->data_items[n].name );
                    free( msg->u.crud.metadata->data_items[n].value );
                    n++;
                }

                free( msg->u.crud.metadata->data_items );
                free( msg->u.crud.metadata );
            }


            break;
        case WRP_MSG_TYPE__AUTH:
        case WRP_MSG_TYPE__SVC_ALIVE:
            break;
        default:
            WRP_ERROR("wrp_free_struct()->Invalid Message Type! (0x%x)\n",
                    msg->msg_type );
            break;
    }

    free( msg );
}

/* See wrp-c.h for details. */
const char *wrp_get_msg_dest (const wrp_msg_t *wrp_msg)
{
	if (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)
		return (const char *)wrp_msg->u.req.dest;
	if (wrp_msg->msg_type == WRP_MSG_TYPE__EVENT)
		return (const char *)wrp_msg->u.event.dest;
	if (wrp_msg->msg_type == WRP_MSG_TYPE__CREATE)
		return (const char *)wrp_msg->u.crud.dest;
	if (wrp_msg->msg_type == WRP_MSG_TYPE__RETREIVE)
		return (const char *)wrp_msg->u.crud.dest;
	if (wrp_msg->msg_type == WRP_MSG_TYPE__UPDATE)
		return (const char *)wrp_msg->u.crud.dest;
	if (wrp_msg->msg_type == WRP_MSG_TYPE__DELETE)
		return (const char *)wrp_msg->u.crud.dest;
	return NULL;
}

/* See wrp-c.h for details. */
const char *wrp_get_msg_source (const wrp_msg_t *wrp_msg)
{
        if (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)
                return (const char *)wrp_msg->u.req.source;
        if (wrp_msg->msg_type == WRP_MSG_TYPE__EVENT)
                return (const char *)wrp_msg->u.event.source;
        if (wrp_msg->msg_type == WRP_MSG_TYPE__CREATE)
                return (const char *)wrp_msg->u.crud.source;
        if (wrp_msg->msg_type == WRP_MSG_TYPE__RETREIVE)
                return (const char *)wrp_msg->u.crud.source;
        if (wrp_msg->msg_type == WRP_MSG_TYPE__UPDATE)
                return (const char *)wrp_msg->u.crud.source;
        if (wrp_msg->msg_type == WRP_MSG_TYPE__DELETE)
                return (const char *)wrp_msg->u.crud.source;
        return NULL;
}

/* See wrp-c.h for details. */
char *wrp_get_msg_element( const enum wrp_device_id_element element,
                                const wrp_msg_t *wrp_msg, const enum wrp_token_name wrp_token )
{
    const char *dest;
    const char *source;
    const char *start = NULL, *end = NULL;
    char *rv = NULL;

	if (wrp_token == DEST) {
		dest = wrp_get_msg_dest(wrp_msg);
		if (NULL != dest ) {
			start = dest;
		}
	} else if(wrp_token == SOURCE) {
		source = wrp_get_msg_source(wrp_msg);
		if (NULL != source ) {
			start = source;
		}
	}
    if (NULL != start ) {
        end = strchr(start, ':');
        if (NULL != end) {
            if (WRP_ID_ELEMENT__SCHEME == element) {
                rv = strdupptr(start, end);
            } else {
                start = end;
                start++;
                end = strchr(start, '/');
                if (NULL != end) {
                    if (WRP_ID_ELEMENT__ID == element) {
                        rv = strdupptr(start, end);
                    } else {
                        start = end;
                        start++;
                        end = strchr(start, '/');
                        if (NULL != end) {
                            if (WRP_ID_ELEMENT__SERVICE == element) {
                                rv = strdupptr(start, end);
                            } else {
                                if (WRP_ID_ELEMENT__APPLICATION == element) {
                                    start = end;
                                    start++;
                                    if (0 < strlen(start)) {
                                        rv = strdup(start);
                                    }
                                }
                            }
                        }
                        else
                        {
							if (WRP_ID_ELEMENT__SERVICE == element) {
								if (0 < strlen(start)) {
									rv = strdup(start);
								}
							 }
                        }
                    }
                }
                else
                {
					if (WRP_ID_ELEMENT__ID == element) {
						if (0 < strlen(start)) {
							rv = strdup(start);
						}
					 }
                }
            }
        }
        else
        {
			if (WRP_ID_ELEMENT__SCHEME == element) {
				if (0 < strlen(start)) {
					rv = strdup(start);
				}
			}
        }
    }

    return rv;
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
    ssize_t rv = -1;
    const struct wrp_req_msg *req = & ( msg->u.req );
    const struct wrp_event_msg *event = & ( msg->u.event );
    const struct wrp_svc_registration_msg *reg = & ( msg->u.reg );
    const struct wrp_crud_msg *crud = &( msg->u.crud );
    const struct wrp_auth_msg *auth = &( msg->u.auth );

    if( NULL == msg || NULL == bytes ) {
        return -1;
    }

    *bytes = NULL;
    struct req_res_t *encode = malloc( sizeof( struct req_res_t ) );

    memset( encode, 0, sizeof( struct req_res_t ) );

    //convert to binary bytes using msgpack
    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            encode->msgType = msg->msg_type;
            encode->statusValue = auth->status;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        case WRP_MSG_TYPE__REQ:
            encode->source = req->source;
            encode->dest = req->dest;
            encode->content_type = req->content_type;
            encode->accept = req->accept;
            encode->transaction_uuid = req->transaction_uuid;
            encode->include_spans = req->include_spans;
            encode->spans = req->spans;
            encode->payload = req->payload;//void
            encode->payload_size = req->payload_size;
            encode->headers = req->headers;
            encode->metadata = req->metadata;
            encode->partner_ids = req->partner_ids;
            encode->msgType = msg->msg_type;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        case WRP_MSG_TYPE__EVENT:
            encode->source = event->source;
            encode->dest = event->dest;
            encode->content_type = event->content_type;
            encode->transaction_uuid = NULL;
            encode->include_spans = false;
            encode->spans.spans = NULL;
            encode->payload = event->payload;//void
            encode->payload_size = event->payload_size;
            encode->headers = event->headers;
            encode->metadata = event->metadata;
            encode->partner_ids = event->partner_ids;
            encode->msgType = msg->msg_type;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        case WRP_MSG_TYPE__SVC_REGISTRATION:
            encode->msgType = msg->msg_type;
            encode->service_name = reg->service_name;
            encode->url = reg->url;
            encode->transaction_uuid = NULL;
            encode->include_spans = false;
            encode->spans.spans = NULL;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            encode->msgType = msg->msg_type;
            encode->source = crud->source;
            encode->dest = crud->dest;
            encode->transaction_uuid = crud->transaction_uuid;
            encode->include_spans = crud->include_spans;
            encode->spans = crud->spans;
            encode->content_type = crud->content_type;
            encode->accept = crud->accept;
            encode->payload = crud->payload;//void
            encode->payload_size = crud->payload_size;
            encode->partner_ids = crud->partner_ids;
            encode->headers = crud->headers;
            encode->metadata = crud->metadata;
            encode->path = crud->path;
            encode->status = crud->status;
            encode->rdr = crud->rdr;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        case WRP_MSG_TYPE__SVC_ALIVE:
            encode->msgType = msg->msg_type;
            rv = __wrp_pack_structure( encode, bytes );
            break;
        default:
            WRP_ERROR("Unknown msgType to encode\n" ); 
            break;  
    }

    free( encode );
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
        return rv;
    }

    bytes_size = ( size_t ) rv;
    base64_buf_size = b64_get_encoded_buffer_size( rv );
    base64_data = ( char* ) malloc( sizeof( char ) * base64_buf_size );

    if( NULL == base64_data ) {
        rv = -101;
    } else {
        b64_encode( ( uint8_t* ) bytes_data, bytes_size, ( uint8_t* ) base64_data );
        *bytes = base64_data;
        rv = base64_buf_size;
    }

    free( bytes_data );
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
        case WRP_MSG_TYPE__SVC_ALIVE:
            return __wrp_keep_alive_to_string (bytes);
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

static ssize_t __wrp_keep_alive_to_string (char **bytes )
{
    const char *keep_alive_fmt =
        "wrp_keep_alive_msg {\n"
        "}\n";
    char *data;
    size_t length;
    length = strlen (keep_alive_fmt);

    if( NULL != bytes ) {
        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            strncpy( data, keep_alive_fmt, length );
            data[length] = '\0';
            *bytes = data;
        } else {
            return -1;
        }
    }

    return length;
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
    size_t length;
    char *headers;
    char *spans;
    char *partner_ids;

    headers = __get_header_string( req->headers );
    spans = __get_spans_string( &req->spans );
    partner_ids = __get_partner_ids_string( req->partner_ids );
    length = snprintf( NULL, 0, req_fmt, req->transaction_uuid, req->source,
                       req->dest, partner_ids, headers, req->content_type,
                       req->accept, ( req->include_spans ? "true" : "false" ),
                       spans, req->payload_size );

    if( NULL != bytes ) {
        char *data;
        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            sprintf( data, req_fmt, req->transaction_uuid, req->source,
                     req->dest, partner_ids, headers, req->content_type,
                     req->accept, ( req->include_spans ? "true" : "false" ),
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
    
    if( __empty_list != partner_ids ) {
        free( partner_ids );
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
    const char *event_fmt = "wrp_event_msg {\n"
                            "    .source           = %s\n"
                            "    .dest             = %s\n"
                            "    .partner_ids      = %s\n"
                            "    .headers          = %s\n"
                            "    .content_type     = %s\n"
                            "    .payload_size     = %zd\n"
                            "}\n";
    size_t length;
    char *headers;
    char *partner_ids;
    
    headers = __get_header_string( event->headers );
    partner_ids = __get_partner_ids_string( event->partner_ids );
    
    length = snprintf( NULL, 0, event_fmt, event->source, event->dest, partner_ids,
                       headers, event->content_type, event->payload_size );

    if( NULL != bytes ) {
        char *data;
        data = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

        if( NULL != data ) {
            sprintf( data, event_fmt, event->source, event->dest, partner_ids, headers, event->content_type,
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
    
    if( __empty_list != partner_ids ) {
        free( partner_ids );
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

        for( i = 0; i < headers->count ; i++ ) {
            length += comma;
            length += strlen( headers->headers[i] );
            comma = 2;
        }

        tmp = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

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

        for( i = 0; i < partner_ids->count ; i++ ) {
            length += comma;
            length += strlen( partner_ids->partner_ids[i] );
            comma = 2;
        }

        tmp = ( char* ) malloc( sizeof( char ) * ( length + 1 ) );   /* +1 for '\0' */

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

static void __msgpack_headers( msgpack_packer *pk, headers_t *headers )
{
    if( NULL != headers ) {
        size_t count = headers->count;
        __msgpack_pack_string( pk, WRP_HEADERS.name, WRP_HEADERS.length );
        msgpack_pack_array( pk, count );
        count = 0;

        while( count < headers->count ) {
            __msgpack_pack_string( pk, headers->headers[count], strlen( headers->headers[count] ) );
            count++;
        }
    }
}

static void __msgpack_partner_ids( msgpack_packer *pk, partners_t *partner_ids )
{
    if( NULL != partner_ids ) {
        size_t count = partner_ids->count;
        __msgpack_pack_string( pk, WRP_PARTNER_IDS.name, WRP_PARTNER_IDS.length );
        msgpack_pack_array( pk, count );
        count = 0;

        while( count < partner_ids->count ) {
            __msgpack_pack_string( pk, partner_ids->partner_ids[count], strlen( partner_ids->partner_ids[count] ) );
            count++;
        }
    }
}

static void __msgpack_spans( msgpack_packer *pk, const struct money_trace_spans *spans )
{
    if( ( NULL != spans ) && ( 0 < spans->count ) ) {
        size_t i;
        struct money_trace_span *span;
        __msgpack_pack_string( pk, WRP_SPANS.name, WRP_SPANS.length );
        msgpack_pack_array( pk, spans->count );
        span = spans->spans;

        for( i = 0; i < spans->count; i++ ) {
            msgpack_pack_array( pk, 3 );
            __msgpack_pack_string( pk, span->name, strlen( span->name ) );
            msgpack_pack_uint64( pk, span->start );
            msgpack_pack_uint32( pk, span->duration );
            span++;
        }
    }
}

static void __msgpack_maps( msgpack_packer *pk, const data_t *dataMap )
{
    size_t i;

    if( dataMap != NULL ) {
        struct data *tmpdata;
        msgpack_pack_map( pk, dataMap->count );
        tmpdata = dataMap->data_items;
        WRP_DEBUG("dataMap->count is %zu\n", dataMap->count );

        for( i = 0; i < dataMap->count; i++ ) {
            struct wrp_token WRP_MAP_NAME;

            WRP_MAP_NAME.name = tmpdata[i].name;
            WRP_MAP_NAME.length = strlen( tmpdata[i].name );
            __msgpack_pack_string_nvp( pk, &WRP_MAP_NAME, tmpdata[i].value );
        }
    } else {
        WRP_ERROR("Map is NULL.Do not pack\n" );
    }
}

static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct wrp_token *token,
                                       const char *val )
{
    if( ( NULL != token ) && ( NULL != val ) ) {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, strlen( val ) );
    }
}

static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

//Pack msgType,source,dest,headers,metadata,partner_ids
static void mapCommonString( msgpack_packer *pk, struct req_res_t *encodeComReq )
{
    __msgpack_pack_string( pk, WRP_MSG_TYPE.name, WRP_MSG_TYPE.length );
    msgpack_pack_int( pk, encodeComReq->msgType );
    __msgpack_pack_string_nvp( pk, &WRP_SOURCE, encodeComReq->source );
    __msgpack_pack_string_nvp( pk, &WRP_DEST, encodeComReq->dest );
    __msgpack_partner_ids( pk, encodeComReq->partner_ids );
    __msgpack_headers( pk, encodeComReq->headers );

    if( encodeComReq->metadata != NULL ) {
        __msgpack_pack_string( pk, WRP_METADATA.name, WRP_METADATA.length );
        __msgpack_maps( pk, encodeComReq->metadata );
    }
}

/**
 *  Encode/Pack the wrp message structure using msgpack
 *
 *  @param  [in]  the messages to convert
 *  @param data [out] the encoded output
 *
 *  @return the number of bytes in the string or less then 1 on error*/


static ssize_t __wrp_pack_structure( struct req_res_t *encodeReq , char **data )

{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    ssize_t rv;
    int wrp_map_size = WRP_MAP_SIZE;
    struct req_res_t *encodeReqtmp =  encodeReq;
    /***   Start of Msgpack Encoding  ***/
    WRP_DEBUG("***   Start of Msgpack Encoding  ***\n" );
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    // Change wrp_map_size value depending on if optional fields spans and headers,metadata,content_type,accept crud payload are present
    if( encodeReqtmp->transaction_uuid ) {
        wrp_map_size++;
    }
    
    if( encodeReqtmp->content_type ) {
        wrp_map_size++;
    }

    if( encodeReqtmp->accept ) {
        wrp_map_size++;
    }

    if( encodeReqtmp->include_spans ) {
        wrp_map_size++;
    }

    if( ( NULL != encodeReqtmp->spans.spans ) && ( 0 < encodeReqtmp->spans.count ) ) {
        wrp_map_size++;
    }

    if( encodeReqtmp->headers ) {
        wrp_map_size++;
    }

    if( encodeReqtmp->metadata != NULL ) {
        wrp_map_size++;
    }
    
    if( encodeReqtmp->partner_ids ) {
        wrp_map_size++;
    }

    switch( encodeReqtmp->msgType ) {
        case WRP_MSG_TYPE__AUTH:
            wrp_map_size = 2;//Hardcoded. Pack for msgType and status alone for auth msg
            msgpack_pack_map( &pk, wrp_map_size );
            __msgpack_pack_string( &pk, WRP_MSG_TYPE.name, WRP_MSG_TYPE.length );
            msgpack_pack_int( &pk, encodeReqtmp->msgType );
            __msgpack_pack_string( &pk, WRP_STATUS.name, WRP_STATUS.length );
            msgpack_pack_int( &pk, encodeReqtmp->statusValue );
            break;
        case WRP_MSG_TYPE__REQ:
            msgpack_pack_map( &pk, wrp_map_size );
            //Pack msgType,source,dest,headers,metadata,partner_ids
            mapCommonString( &pk, encodeReqtmp );
            __msgpack_pack_string_nvp( &pk, &WRP_TRANS_ID, encodeReqtmp->transaction_uuid );
            __msgpack_pack_string_nvp( &pk, &WRP_CONTENT_TYPE, encodeReqtmp->content_type );
            if( encodeReqtmp->accept ) {
                __msgpack_pack_string_nvp( &pk, &WRP_ACCEPT, encodeReqtmp->accept );
            }

            if( encodeReqtmp->include_spans ) {
                __msgpack_pack_string( &pk, WRP_INCLUDE_SPANS.name, WRP_INCLUDE_SPANS.length );
                msgpack_pack_true( &pk );
            }

            __msgpack_spans( &pk, &encodeReqtmp->spans );
            __msgpack_pack_string( &pk, WRP_PAYLOAD.name, WRP_PAYLOAD.length );
            msgpack_pack_bin( &pk, encodeReqtmp->payload_size );
            msgpack_pack_bin_body( &pk, encodeReqtmp->payload, encodeReqtmp->payload_size );
            break;
        case WRP_MSG_TYPE__EVENT:
            msgpack_pack_map( &pk, wrp_map_size );
            //Pack msgType,source,dest,headers,metadata,partner_ids
            mapCommonString( &pk, encodeReqtmp );
            __msgpack_pack_string_nvp( &pk, &WRP_CONTENT_TYPE, encodeReqtmp->content_type );
            __msgpack_pack_string( &pk, WRP_PAYLOAD.name, WRP_PAYLOAD.length );
            msgpack_pack_bin( &pk, encodeReqtmp->payload_size );
            msgpack_pack_bin_body( &pk, encodeReqtmp->payload, encodeReqtmp->payload_size );
            break;
        case WRP_MSG_TYPE__SVC_REGISTRATION:
            wrp_map_size = 3;//Hardcoded.Pack service name and url only
            msgpack_pack_map( &pk, wrp_map_size );
            __msgpack_pack_string( &pk, WRP_MSG_TYPE.name, WRP_MSG_TYPE.length );
            msgpack_pack_int( &pk, encodeReqtmp->msgType );
            __msgpack_pack_string_nvp( &pk, &WRP_SERVICE_NAME, encodeReqtmp->service_name );
            __msgpack_pack_string_nvp( &pk, &WRP_URL, encodeReqtmp->url );
            break;
        case WRP_MSG_TYPE__SVC_ALIVE:
            wrp_map_size = 1;//Hardcoded. Pack for msgType only
            msgpack_pack_map( &pk, wrp_map_size );
            __msgpack_pack_string( &pk, WRP_MSG_TYPE.name, WRP_MSG_TYPE.length );
            msgpack_pack_int( &pk, encodeReqtmp->msgType );
            break;
        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:

            if( encodeReqtmp->payload == NULL ) {
                wrp_map_size--;
                WRP_INFO("CRUD payload is NULL map size is %d\n", wrp_map_size );
            }

            if( encodeReqtmp->path != NULL ) {
                wrp_map_size++;
            }

            if( encodeReqtmp->status != 0 ) {
                wrp_map_size++; //status
            }

            if( encodeReqtmp->rdr >= 0 ) {
                wrp_map_size++; //rdr
            }

            msgpack_pack_map( &pk, wrp_map_size );
            //Pack msgType,source,dest,headers,metadata,partner_ids
            mapCommonString( &pk, encodeReqtmp );
            __msgpack_pack_string_nvp( &pk, &WRP_TRANS_ID, encodeReqtmp->transaction_uuid );

            if( encodeReqtmp->include_spans ) {
                __msgpack_pack_string( &pk, WRP_INCLUDE_SPANS.name, WRP_INCLUDE_SPANS.length );
                msgpack_pack_true( &pk );
            }

            __msgpack_spans( &pk, &encodeReqtmp->spans );

            if( encodeReqtmp->status != 0 ) {
                __msgpack_pack_string( &pk, WRP_STATUS.name, WRP_STATUS.length );
                msgpack_pack_int( &pk, encodeReqtmp->status );
            }

            if( encodeReqtmp->rdr >= 0 ) {
                __msgpack_pack_string( &pk, WRP_RDR.name, WRP_RDR.length );
                msgpack_pack_int( &pk, encodeReqtmp->rdr );
            }

            if( encodeReqtmp->path != NULL ) {
                __msgpack_pack_string_nvp( &pk, &WRP_PATH, encodeReqtmp->path );
            }
            
            if( encodeReqtmp->content_type != NULL) {
                __msgpack_pack_string_nvp( &pk, &WRP_CONTENT_TYPE, encodeReqtmp->content_type );
            }

            if( encodeReqtmp->accept != NULL) {
                __msgpack_pack_string_nvp( &pk, &WRP_ACCEPT, encodeReqtmp->accept );
            }

            if( encodeReqtmp->payload != NULL ) {
                __msgpack_pack_string( &pk, WRP_PAYLOAD.name, WRP_PAYLOAD.length );
                msgpack_pack_bin( &pk, encodeReqtmp->payload_size );
                msgpack_pack_bin_body( &pk, encodeReqtmp->payload, encodeReqtmp->payload_size );
            }

            break;
        default:
            WRP_ERROR("Un-supported format to pack\n" );
            return -1;
    }

    rv = -1;

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}

/**
 *  Encode/pack only metadata from wrp_msg_t structure.
 *
 *  @note Do not call free of output data in failure case!
 *
 *  @param msg [in] packData the data_t structure to pack/encode
 *  @param msg [out] the encoded output
 *  @return encoded buffer size or less than 1 in failure case
 */

ssize_t wrp_pack_metadata( const data_t *packData, void **data )
{
    size_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    if( packData != NULL && packData->count != 0 ) {
        __msgpack_pack_string( &pk, WRP_METADATA.name, WRP_METADATA.length );
        __msgpack_maps( &pk, packData );
    } else {
        WRP_ERROR("Metadata is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
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
 * @param[in] include_spans to include timing values
 */

static void decodeRequest( msgpack_object deserialized, struct req_res_t **decodeReq )
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
    char *source = NULL;
    char *dest = NULL;
    char *payload = NULL;
    char *service_name = NULL;
    char *url = NULL;
    char *path = NULL;
    char *content_type = NULL;
    char *accept = NULL;
    struct req_res_t *tmpdecodeReq = *decodeReq;
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
                        if( strcmp( keyName, WRP_MSG_TYPE.name ) == 0 ) {
                            tmpdecodeReq->msgType = ValueType.via.i64;
                        } else if( strcmp( keyName, WRP_STATUS.name ) == 0 ) {
                            tmpdecodeReq->statusValue = ValueType.via.i64;
                        } else if( strcmp( keyName, WRP_RDR.name ) == 0 ) {
                            tmpdecodeReq->rdr = ValueType.via.i64;
                        }
                    }
                    break;
                    case MSGPACK_OBJECT_BOOLEAN: {
                        if( strcmp( keyName, WRP_INCLUDE_SPANS.name ) == 0 ) {
                            tmpdecodeReq->include_spans = ValueType.via.boolean ? true : false;
                        }
                    }
                    break;
                    case MSGPACK_OBJECT_STR: {
                        StringValueSize = ValueType.via.str.size;
                        NewStringVal = ( char* ) malloc( StringValueSize + 1 );
                        StringValue = getKey_MsgtypeStr( ValueType, StringValueSize, NewStringVal );

                        if( strcmp( keyName, WRP_SOURCE.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            source = ( char * ) malloc( sLen + 1 );
                            strncpy( source, StringValue, sLen );
                            source[sLen] = '\0';
                            tmpdecodeReq->source = source;
                        } else if( strcmp( keyName, WRP_DEST.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            dest = ( char * ) malloc( sLen + 1 );
                            strncpy( dest, StringValue, sLen );
                            dest[sLen] = '\0';
                            tmpdecodeReq->dest = dest;
                        } else if( strcmp( keyName, WRP_TRANS_ID.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            transaction_uuid = ( char * ) malloc( sLen + 1 );
                            strncpy( transaction_uuid, StringValue, sLen );
                            transaction_uuid[sLen] = '\0';
                            tmpdecodeReq->transaction_uuid = transaction_uuid;
                        } else if( strcmp( keyName, WRP_SERVICE_NAME.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            service_name = ( char * ) malloc( sLen + 1 );
                            strncpy( service_name, StringValue, sLen );
                            service_name[sLen] = '\0';
                            tmpdecodeReq->service_name = service_name;
                        } else if( strcmp( keyName, WRP_URL.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            url = ( char * ) malloc( sLen + 1 );
                            strncpy( url, StringValue, sLen );
                            url[sLen] = '\0';
                            tmpdecodeReq->url = url;
                        } else if( strcmp( keyName, WRP_HEADERS.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            tmpdecodeReq->headers = ( headers_t * ) malloc( sizeof( headers_t )
                                                    + sizeof( char * ) * 1 );
                            tmpdecodeReq->headers->count = 1;
                            tmpdecodeReq->headers->headers[0] = ( char * ) malloc( sLen );
                            memset( tmpdecodeReq->headers->headers, 0, sLen );
                            strncpy( tmpdecodeReq->headers->headers[0], StringValue, sLen );
                        } else if( strcmp( keyName, WRP_PATH.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            path = ( char * ) malloc( sLen + 1 );
                            strncpy( path, StringValue, sLen );
                            path[sLen] = '\0';
                            tmpdecodeReq->path = path;
                        } else if( strcmp( keyName, WRP_CONTENT_TYPE.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            content_type = ( char * ) malloc( sLen + 1 );
                            strncpy( content_type, StringValue, sLen );
                            content_type[sLen] = '\0';
                            tmpdecodeReq->content_type = content_type;
                        } else if( strcmp( keyName, WRP_ACCEPT.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            accept = ( char * ) malloc( sLen + 1 );
                            strncpy( accept, StringValue, sLen );
                            accept[sLen] = '\0';
                            tmpdecodeReq->accept = accept;
                        } else if( strcmp( keyName, WRP_PARTNER_IDS.name ) == 0 ) {
                            sLen = strlen( StringValue );
                            tmpdecodeReq->partner_ids = ( partners_t * ) malloc( sizeof( partners_t )
                                                    + sizeof( char * ) * 1 );
                            tmpdecodeReq->partner_ids->count = 1;
                            tmpdecodeReq->partner_ids->partner_ids[0] = ( char * ) malloc( sLen );
                            memset( tmpdecodeReq->partner_ids->partner_ids, 0, sLen );
                            strncpy( tmpdecodeReq->partner_ids->partner_ids[0], StringValue, sLen );
                         }

                        free( NewStringVal );
                    }
                    break;
                    case MSGPACK_OBJECT_BIN: {
                        if( strcmp( keyName, WRP_PAYLOAD.name ) == 0 ) {
                            binValueSize = ValueType.via.bin.size;
                            payload = ( char* ) malloc( binValueSize + 1 );
                            memset( payload, 0, binValueSize + 1 );
                            keyValue = NULL;
                            keyValue = getKey_MsgtypeBin( ValueType, binValueSize, payload );

                            if( keyValue != NULL ) {
                                WRP_DEBUG("Binary payload %s\n", keyValue );
                            }

                            tmpdecodeReq->payload = keyValue;
                            tmpdecodeReq->payload_size = binValueSize;
                        }
                    }
                    break;
                    case MSGPACK_OBJECT_ARRAY:

                        if( strcmp( keyName, WRP_HEADERS.name ) == 0 ) {
                            msgpack_object_array array = ValueType.via.array;
                            msgpack_object *ptr = array.ptr;
                            uint32_t cnt = 0;
                            ptr = array.ptr;
                            tmpdecodeReq->headers = ( headers_t * ) malloc( sizeof( headers_t )
                                                    + sizeof( char * ) * array.size );
                            tmpdecodeReq->headers->count = array.size;

                            for( cnt = 0; cnt < array.size; cnt++, ptr++ ) {
                                tmpdecodeReq->headers->headers[cnt] = ( char * ) malloc( ptr->via.str.size + 1 );
                                memset( tmpdecodeReq->headers->headers[cnt], 0, ptr->via.str.size + 1 );
                                memcpy( tmpdecodeReq->headers->headers[cnt], ptr->via.str.ptr, ptr->via.str.size );
                                WRP_DEBUG("tmpdecodeReq->headers[%d] %s\n", cnt, tmpdecodeReq->headers->headers[cnt] );
                            }
                        }else if( strcmp( keyName, WRP_PARTNER_IDS.name ) == 0 ) {
                            msgpack_object_array array = ValueType.via.array;
                            msgpack_object *ptr = array.ptr;
                            uint32_t cnt = 0;
                            ptr = array.ptr;
                            tmpdecodeReq->partner_ids = ( partners_t * ) malloc( sizeof( partners_t )
                                                    + sizeof( char * ) * array.size );
                            tmpdecodeReq->partner_ids->count = array.size;

                            for( cnt = 0; cnt < array.size; cnt++, ptr++ ) {
                                tmpdecodeReq->partner_ids->partner_ids[cnt] = ( char * ) malloc( ptr->via.str.size + 1 );
                                memset( tmpdecodeReq->partner_ids->partner_ids[cnt], 0, ptr->via.str.size + 1 );
                                memcpy( tmpdecodeReq->partner_ids->partner_ids[cnt], ptr->via.str.ptr, ptr->via.str.size );
                                WRP_DEBUG("tmpdecodeReq->partner_ids[%d] %s\n", cnt, tmpdecodeReq->partner_ids->partner_ids[cnt] );
                            }
                        } else {
                            WRP_ERROR("Not Handled MSGPACK_OBJECT_ARRAY %s\n", keyName );
                        }

                        break;
                    case MSGPACK_OBJECT_MAP:
                        WRP_DEBUG("Type of MAP\n" );
                        WRP_DEBUG("keyName is %s\n",keyName);
                        decodeMapRequest( ValueType, decodeReq );
                        break;
                    case MSGPACK_OBJECT_NIL:

                        if( strcmp( keyName, WRP_SPANS.name ) == 0 ) {
                            WRP_DEBUG("spans is nil\n" );
                        }

                        break;
                    default:
                        WRP_ERROR("Unknown Data Type\n" );
                        break;
                }
        }

        p++;
        i++;
        free( keyString );
    }
}

static void decodeMapRequest( msgpack_object deserialized, struct req_res_t **decodeMapReq )
{
    unsigned int i = 0;
    int n = 0, v = 0;
    int keySize = 0;
    char* keyString = NULL;
    char* keyName = NULL;
    int sLen = 0, kLen = 0;
    int StringValueSize = 0;
    char* NewStringVal, *StringValue;
    char* mapName = NULL;
    char *mapValue = NULL;
    struct req_res_t *mapdecodeReq = *decodeMapReq;
    msgpack_object_kv* p = deserialized.via.map.ptr;
    WRP_DEBUG("Map size is %d\n", deserialized.via.map.size );

    WRP_DEBUG("mapdecodeReq->metadata->count is %d\n", deserialized.via.map.size );

    if( deserialized.via.map.size != 0 ) {
        mapdecodeReq->metadata->count = deserialized.via.map.size;
        mapdecodeReq->metadata->data_items = ( struct data* )malloc( sizeof( struct data ) * ( deserialized.via.map.size ) );
    }

    while( i < deserialized.via.map.size ) {
        sLen = 0;
        msgpack_object keyType = p->key;
        msgpack_object ValueType = p->val;
        keySize = keyType.via.str.size;
        keyString = ( char* ) malloc( keySize + 1 );
        keyName = NULL;
        keyName = getKey_MsgtypeStr( keyType, keySize, keyString );

        if( keyName != NULL ) {
            kLen = strlen( keyName );
            mapName = ( char * ) malloc( kLen + 1 );
            strncpy( mapName, keyName, kLen );
            mapName[kLen] = '\0';
            mapdecodeReq->metadata->data_items[n].name = mapName;
            n++;

            switch( ValueType.type ) {
                case MSGPACK_OBJECT_POSITIVE_INTEGER: {
                    WRP_DEBUG("Map value is int %" PRId64 "\n", ValueType.via.i64 );
                    sprintf( mapdecodeReq->metadata->data_items[v].value, "%" PRId64, ValueType.via.i64 );
                    v++;
                }
                break;
                case MSGPACK_OBJECT_BOOLEAN: {
                    WRP_DEBUG("Map value boolean %d\n", ValueType.via.boolean ? true : false );
                    mapdecodeReq->metadata->data_items[v].value = ValueType.via.boolean ? "true" : "false";
                    v++;
                }
                break;
                case MSGPACK_OBJECT_STR: {
                    StringValueSize = ValueType.via.str.size;
                    NewStringVal = ( char* ) malloc( StringValueSize + 1 );
                    StringValue = getKey_MsgtypeStr( ValueType, StringValueSize, NewStringVal );
                    sLen = strlen( StringValue );
                    mapValue = ( char * ) malloc( sLen + 1 );
                    strncpy( mapValue, StringValue, sLen );
                    mapValue[sLen] = '\0';
                    mapdecodeReq->metadata->data_items[v].value = mapValue;
                    v++;
                    free( NewStringVal );
                }
                break;
                default:
                    WRP_ERROR("Unknown data format inside MAP\n" );
                    break;
            }
        }

        p++;
        i++;
        free( keyString );
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
    wrp_msg_t *msg = NULL;

    if( bytes != NULL ) 
    {
        struct req_res_t *decodeReq = malloc( sizeof( struct req_res_t ) );
        memset( decodeReq, 0, sizeof( struct req_res_t ) );
        decodeReq->metadata = malloc( sizeof( data_t ) );
        memset( decodeReq->metadata, 0, sizeof( data_t ) );
        WRP_DEBUG("unpacking encoded data\n" );
        msgpack_zone_init( &mempool, 2048 );
        unpack_ret = msgpack_unpack( bytes, length, NULL, &mempool, &deserialized );
        WRP_DEBUG("unpack_ret:%d\n", unpack_ret );

        switch( unpack_ret ) {
            case MSGPACK_UNPACK_SUCCESS: {
                //msgpack_object_print( stdout, deserialized );
                //puts("");
                if( deserialized.via.map.size != 0 ) {
                    decodeRequest( deserialized, &decodeReq );
                }

                msgpack_zone_destroy( &mempool );
                msg = ( wrp_msg_t * ) malloc( sizeof( wrp_msg_t ) );
                memset( msg, 0, sizeof( wrp_msg_t ) );

                switch( decodeReq->msgType ) {
                    case WRP_MSG_TYPE__AUTH:
                        msg->msg_type = decodeReq->msgType;
                        msg->u.auth.status = decodeReq->statusValue;
                        *msg_ptr = msg;
                        free( decodeReq->metadata->data_items );
                        free( decodeReq->metadata );
                        free( decodeReq );
                        return length;
                    case WRP_MSG_TYPE__REQ:
                        msg->msg_type = decodeReq->msgType;
                        msg->u.req.source = decodeReq->source;
                        msg->u.req.dest = decodeReq->dest;
                        msg->u.req.transaction_uuid = decodeReq->transaction_uuid;
                        msg->u.req.content_type = decodeReq->content_type;
                        msg->u.req.accept = decodeReq->accept;
                        msg->u.req.headers = decodeReq->headers;
                        msg->u.req.metadata = decodeReq->metadata;
                        msg->u.req.include_spans = decodeReq->include_spans;
                        msg->u.req.spans.spans = NULL;   /* not supported */
                        msg->u.req.spans.count = 0;     /* not supported */
                        msg->u.req.payload = decodeReq->payload;
                        msg->u.req.payload_size = decodeReq->payload_size;
                        msg->u.req.partner_ids = decodeReq->partner_ids;
                        *msg_ptr = msg;
                        free( decodeReq );
                        return length;
                    case WRP_MSG_TYPE__EVENT:
                        msg->msg_type = decodeReq->msgType;
                        msg->u.event.source = decodeReq->source;
                        msg->u.event.dest = decodeReq->dest;
                        msg->u.event.content_type = decodeReq->content_type;
                        msg->u.event.metadata = decodeReq->metadata;
                        msg->u.event.payload = decodeReq->payload;
                        msg->u.event.payload_size = decodeReq->payload_size;
                        msg->u.event.headers = decodeReq->headers;
                        msg->u.event.partner_ids = decodeReq->partner_ids;
                        *msg_ptr = msg;
                        free( decodeReq );
                        return length;
                    case WRP_MSG_TYPE__SVC_REGISTRATION:
                        msg->msg_type = decodeReq->msgType;
                        msg->u.reg.service_name = decodeReq->service_name;
                        msg->u.reg.url = decodeReq->url;
                        *msg_ptr = msg;
                        free( decodeReq->metadata->data_items );
                        free( decodeReq->metadata );
                        free( decodeReq );
                        return length;
                    case WRP_MSG_TYPE__SVC_ALIVE:
                        msg->msg_type = decodeReq->msgType;
                        *msg_ptr = msg;
                        free( decodeReq->metadata );
                        free( decodeReq );
                        return length;
                    case WRP_MSG_TYPE__CREATE:
                    case WRP_MSG_TYPE__RETREIVE:
                    case WRP_MSG_TYPE__UPDATE:
                    case WRP_MSG_TYPE__DELETE:
                        msg->msg_type = decodeReq->msgType;
                        msg->u.crud.source = decodeReq->source;
                        msg->u.crud.dest = decodeReq->dest;
                        msg->u.crud.transaction_uuid = decodeReq->transaction_uuid;
                        msg->u.crud.partner_ids = decodeReq->partner_ids;
                        msg->u.crud.headers = decodeReq->headers;
                        msg->u.crud.metadata = decodeReq->metadata;
                        msg->u.crud.include_spans = decodeReq->include_spans;
                        msg->u.crud.content_type = decodeReq->content_type;
                        msg->u.crud.accept = decodeReq->accept;
                        msg->u.crud.spans.spans = NULL;   /* not supported */
                        msg->u.crud.spans.count = 0;     /* not supported */
                        msg->u.crud.status = decodeReq->statusValue;
                        msg->u.crud.rdr = decodeReq->rdr;
                        msg->u.crud.payload = decodeReq->payload;
                        msg->u.crud.payload_size = decodeReq->payload_size;
                        msg->u.crud.path = decodeReq->path;
                        free( decodeReq );
                        *msg_ptr = msg;
                        return length;
                    default:
                        free( decodeReq->metadata->data_items );
                        free( decodeReq->metadata );
                        free( decodeReq );
                        free(msg);
                        return -1;
                }
                break; 
            }

            case MSGPACK_UNPACK_EXTRA_BYTES: {
                WRP_ERROR("MSGPACK_UNPACK_EXTRA_BYTES\n" );
		msgpack_zone_destroy( &mempool );
                free( decodeReq->metadata );
                free( decodeReq );
                return -1;
            }
            case MSGPACK_UNPACK_CONTINUE: {
                WRP_ERROR("MSGPACK_UNPACK_CONTINUE\n" );
		msgpack_zone_destroy( &mempool );
                free( decodeReq->metadata );
                free( decodeReq );
                return -1;
            }
            case MSGPACK_UNPACK_PARSE_ERROR: {
                WRP_ERROR("MSGPACK_UNPACK_PARSE_ERROR\n" );
		msgpack_zone_destroy( &mempool );
                free( decodeReq->metadata );
                free( decodeReq );
                return -1;
            }
            case MSGPACK_UNPACK_NOMEM_ERROR: {
                WRP_ERROR("MSGPACK_UNPACK_NOMEM_ERROR\n" );
		msgpack_zone_destroy( &mempool );
                free( decodeReq->metadata );
                free( decodeReq );
                return -1;
            }
            default:
		msgpack_zone_destroy( &mempool );
                free( decodeReq->metadata );
                free( decodeReq );
                return -1;
        }
    }

    WRP_ERROR("bytes is NULL\n" );
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
/**
 * @brief alterMap function to change MAP size of encoded msgpack object.
 *
 * @param[in] encodedBuffer msgpack object
 * @param[out] return 0 in success or less than 1 in failure case
 */

static int alterMap( char * buf )
{
    //Extract 1st byte from binary stream which holds type and map size
    unsigned char *byte = ( unsigned char * )( &( buf[0] ) ) ;
    int mapSize;
    WRP_DEBUG("First byte in hex : %x\n", 0xff & *byte );
    //Calculate map size
    mapSize = ( 0xff & *byte ) % 0x10;
    WRP_DEBUG("Map size is :%d\n", mapSize );

    if( mapSize == 15 ) {
        WRP_ERROR("Msgpack Map (fixmap) is already at its MAX size i.e. 15\n" );
        return -1;
    }

    *byte = *byte + METADATA_MAP_SIZE;
    mapSize = ( 0xff & *byte ) % 0x10;
    WRP_DEBUG("New Map size : %d\n", mapSize );
    //Update 1st byte with new MAP size
    buf[0] = *byte;
    return 0;
}

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

size_t appendEncodedData( void **appendData, void *encodedBuffer, size_t encodedSize, void *metadataPack, size_t metadataSize )
{
    //Allocate size for final buffer
    *appendData = ( void * )malloc( sizeof( char * ) * ( encodedSize + metadataSize ) );
    memcpy( *appendData, encodedBuffer, encodedSize );
    //Append 2nd encoded buf with 1st encoded buf
    memcpy( *appendData + ( encodedSize ), metadataPack, metadataSize );
    //Alter MAP
    int ret = alterMap( ( char * ) * appendData );

    if( ret ) {
        return -1;
    }

    return ( encodedSize + metadataSize );
}

/**
 *  @brief Helper function that copies a portion of a string defined by pointers
 *
 *  @param s the start of the string to copy
 *  @param e the last character of the string to copy
 *
 *  @return the allocated buffer with the substring
 */
static char* strdupptr( const char *s, const char *e )
{
    if (s == e) {
        return NULL;
    }

    return strndup(s, (size_t) (((uintptr_t)e) - ((uintptr_t)s)));
}
