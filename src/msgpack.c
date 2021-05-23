/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cimplog/cimplog.h>
#include <msgpack.h>

#include "utils.h"
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
    int msgType;
    int statusValue;
    char* source;
    char* dest;
    char* transaction_uuid;
    partners_t *partner_ids;
    headers_t *headers;
    void *payload;
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
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct wrp_token *token,
                                       const char *val );
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static void decodeRequest( msgpack_object deserialized, struct req_res_t **decodeReq );
static char* getKey_MsgtypeStr( const msgpack_object key, size_t keySize,
                                char* keyString );
static char* getKey_MsgtypeBin( const msgpack_object key, const size_t binSize,
                                char* keyBin );
static void __msgpack_maps( msgpack_packer *pk, const data_t *dataMap );
static void decodeMapRequest( msgpack_object deserialized,
                              struct req_res_t **decodeMapReq );
static void mapCommonString( msgpack_packer *pk, struct req_res_t *encodeComReq );
static ssize_t __wrp_pack_structure( struct req_res_t *encodeReq, char **data );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

ssize_t wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes )
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
    struct req_res_t *encode = calloc( 1, sizeof(struct req_res_t) );

    if( encode != NULL ) {

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
                WRP_ERROR( "Unknown msgType to encode\n" );
                break;
        }

        free( encode );
        return rv;
    } else {
        WRP_ERROR( "Memory allocation failed\n" );
    }

    return -1;
}


ssize_t wrp_bytes_to_struct( const void *bytes, size_t length,
                                      wrp_msg_t **msg_ptr )
{
    struct req_res_t *decodeReq;
    wrp_msg_t *msg;
    struct req_res_t *metadata;

    if( NULL == bytes ) {
        WRP_ERROR( "bytes is NULL\n" );
        return -1;
    }

    decodeReq = calloc( 1, sizeof(struct req_res_t) );
    msg       = calloc( 1, sizeof(wrp_msg_t) );
    metadata  = calloc( 1, sizeof(data_t) );

    if( ( decodeReq != NULL ) && ( NULL != msg ) && ( NULL != metadata ) ) {
        msgpack_zone mempool;
        msgpack_object deserialized;
        msgpack_unpack_return unpack_ret;

        decodeReq->metadata = ( struct data_struct * ) metadata;
        WRP_DEBUG( "unpacking encoded data\n" );
        msgpack_zone_init( &mempool, 2048 );
        unpack_ret = msgpack_unpack( bytes, length, NULL, &mempool, &deserialized );
        WRP_DEBUG( "unpack_ret:%d\n", unpack_ret );

        switch( unpack_ret ) {
            case MSGPACK_UNPACK_SUCCESS:
                if( deserialized.via.map.size != 0 ) {
                    decodeRequest( deserialized, &decodeReq );
                }

                msgpack_zone_destroy( &mempool );

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
                        break;
                }

                break;

            case MSGPACK_UNPACK_EXTRA_BYTES:
                WRP_ERROR( "MSGPACK_UNPACK_EXTRA_BYTES\n" );
                break;

            case MSGPACK_UNPACK_CONTINUE:
                WRP_ERROR( "MSGPACK_UNPACK_CONTINUE\n" );
                break;

            case MSGPACK_UNPACK_PARSE_ERROR:
                WRP_ERROR( "MSGPACK_UNPACK_PARSE_ERROR\n" );
                break;

            case MSGPACK_UNPACK_NOMEM_ERROR:
                WRP_ERROR( "MSGPACK_UNPACK_NOMEM_ERROR\n" );
                break;

            default:
                break;
        }

        msgpack_zone_destroy( &mempool );
    } else {
        WRP_ERROR( "Memory allocation failed\n" );
    }

    if( NULL != decodeReq ) {
        free( decodeReq );
    }

    if( NULL != msg ) {
        free( msg );
    }

    if( NULL != metadata ) {
        free( metadata );
    }

    return -1;
}


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

static void __msgpack_headers( msgpack_packer *pk, const headers_t *headers )
{
    if( headers && (0 < headers->count) ) {
        __msgpack_pack_string( pk, WRP_HEADERS.name, WRP_HEADERS.length );
        msgpack_pack_array( pk, headers->count );

        for( size_t i = 0; i < headers->count; i++ ) {
            __msgpack_pack_string( pk, headers->headers[i], strlen(headers->headers[i]) );
        }
    }
}


static void __msgpack_partner_ids( msgpack_packer *pk, const partners_t *partner_ids )
{
    if( partner_ids && (0 < partner_ids->count) ) {
        __msgpack_pack_string( pk, WRP_PARTNER_IDS.name, WRP_PARTNER_IDS.length );
        msgpack_pack_array( pk, partner_ids->count );

        for( size_t i = 0; i < partner_ids->count; i++ ) {
            __msgpack_pack_string( pk, partner_ids->partner_ids[i],
                                   strlen(partner_ids->partner_ids[i]) );
        }
    }
}


static void __msgpack_spans( msgpack_packer *pk, const struct money_trace_spans *spans )
{
    if( spans && (0 < spans->count) ) {
        struct money_trace_span *span = spans->spans;

        __msgpack_pack_string( pk, WRP_SPANS.name, WRP_SPANS.length );
        msgpack_pack_array( pk, spans->count );

        for( size_t i = 0; i < spans->count; i++ ) {
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
    if( dataMap != NULL ) {
        const struct data *tmpdata;
        msgpack_pack_map( pk, dataMap->count );
        tmpdata = dataMap->data_items;
        WRP_DEBUG( "dataMap->count is %zu\n", dataMap->count );

        for( size_t i = 0; i < dataMap->count; i++ ) {
            struct wrp_token WRP_MAP_NAME;

            WRP_MAP_NAME.name = tmpdata[i].name;
            WRP_MAP_NAME.length = strlen( tmpdata[i].name );
            __msgpack_pack_string_nvp( pk, &WRP_MAP_NAME, tmpdata[i].value );
        }
    } else {
        WRP_ERROR( "Map is NULL.Do not pack\n" );
    }
}


static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct wrp_token *token,
                                       const char *val )
{
    if( token && val ) {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, strlen(val) );
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
 *  @return the number of bytes in the string or less then 1 on error
 */
static ssize_t __wrp_pack_structure( struct req_res_t *encodeReq, char **data )

{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    ssize_t rv;
    int wrp_map_size = WRP_MAP_SIZE;
    struct req_res_t *encodeReqtmp =  encodeReq;
    /***   Start of Msgpack Encoding  ***/
    WRP_DEBUG( "***   Start of Msgpack Encoding  ***\n" );
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
                WRP_INFO( "CRUD payload is NULL map size is %d\n", wrp_map_size );
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

            if( encodeReqtmp->content_type != NULL ) {
                __msgpack_pack_string_nvp( &pk, &WRP_CONTENT_TYPE, encodeReqtmp->content_type );
            }

            if( encodeReqtmp->accept != NULL ) {
                __msgpack_pack_string_nvp( &pk, &WRP_ACCEPT, encodeReqtmp->accept );
            }

            if( encodeReqtmp->payload != NULL ) {
                __msgpack_pack_string( &pk, WRP_PAYLOAD.name, WRP_PAYLOAD.length );
                msgpack_pack_bin( &pk, encodeReqtmp->payload_size );
                msgpack_pack_bin_body( &pk, encodeReqtmp->payload, encodeReqtmp->payload_size );
            }

            break;

        default:
            WRP_ERROR( "Un-supported format to pack\n" );
            return -1;
    }

    rv = -1;

    if( sbuf.data ) {
        *data = malloc( sizeof(char) * sbuf.size );

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
        WRP_ERROR( "Metadata is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = malloc( sizeof(char) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
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
    char *headers = NULL;
    char *partnerID = NULL;
    char *accept = NULL;
    struct req_res_t *tmpdecodeReq = *decodeReq;
    msgpack_object_kv* p = deserialized.via.map.ptr;

    while( i < deserialized.via.map.size ) {
        msgpack_object keyType = p->key;
        msgpack_object ValueType = p->val;
        keySize = keyType.via.str.size;
        keyString = malloc( keySize + 1 );

        if( keyString != NULL ) {
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
                        NewStringVal = malloc( StringValueSize + 1 );

                        if( NewStringVal != NULL ) {
                            StringValue = getKey_MsgtypeStr( ValueType, StringValueSize, NewStringVal );

                            if( strcmp( keyName, WRP_SOURCE.name ) == 0 ) {
                                source = wrp_strdup( StringValue );

                                if( source ) {
                                    tmpdecodeReq->source = source;
                                } else {
                                    tmpdecodeReq->source = NULL;
                                }
                            } else if( strcmp( keyName, WRP_DEST.name ) == 0 ) {
                                dest = wrp_strdup( StringValue );

                                if( dest ) {
                                    tmpdecodeReq->dest = dest;
                                } else {
                                    tmpdecodeReq->dest = NULL;
                                }
                            } else if( strcmp( keyName, WRP_TRANS_ID.name ) == 0 ) {
                                transaction_uuid = wrp_strdup( StringValue );

                                if( transaction_uuid ) {
                                    tmpdecodeReq->transaction_uuid = transaction_uuid;
                                } else {
                                    tmpdecodeReq->transaction_uuid = NULL;
                                }
                            } else if( strcmp( keyName, WRP_SERVICE_NAME.name ) == 0 ) {
                                service_name = wrp_strdup( StringValue );

                                if( service_name ) {
                                    tmpdecodeReq->service_name = service_name;
                                } else {
                                    tmpdecodeReq->service_name = NULL;
                                }
                            } else if( strcmp( keyName, WRP_URL.name ) == 0 ) {
                                url = wrp_strdup( StringValue );

                                if( url ) {
                                    tmpdecodeReq->url = url;
                                } else {
                                    tmpdecodeReq->url = NULL;
                                }
                            } else if( strcmp( keyName, WRP_HEADERS.name ) == 0 ) {
                                tmpdecodeReq->headers = malloc( sizeof(headers_t)
                                                        + sizeof( char * ) * 1 );

                                if( tmpdecodeReq->headers != NULL ) {
                                    headers = wrp_strdup( StringValue );

                                    if( headers ) {
                                        tmpdecodeReq->headers->count = 1;
                                        tmpdecodeReq->headers->headers[0] = headers;
                                    } else {
                                        tmpdecodeReq->headers->count = 1;
                                        tmpdecodeReq->headers->headers[0] = NULL;
                                    }
                                } else {
                                    WRP_ERROR( "Memory allocation failed\n" );
                                }
                            } else if( strcmp( keyName, WRP_PATH.name ) == 0 ) {
                                path = wrp_strdup( StringValue );

                                if( path ) {
                                    tmpdecodeReq->path = path;
                                } else {
                                    tmpdecodeReq->path = NULL;
                                }
                            } else if( strcmp( keyName, WRP_CONTENT_TYPE.name ) == 0 ) {
                                content_type = wrp_strdup( StringValue );

                                if( content_type ) {
                                    tmpdecodeReq->content_type = content_type;
                                } else {
                                    tmpdecodeReq->content_type = NULL;
                                }
                            } else if( strcmp( keyName, WRP_ACCEPT.name ) == 0 ) {
                                accept = wrp_strdup( StringValue );

                                if( accept ) {
                                    tmpdecodeReq->accept = accept;
                                } else {
                                    tmpdecodeReq->accept = NULL;
                                }
                            } else if( strcmp( keyName, WRP_PARTNER_IDS.name ) == 0 ) {
                                tmpdecodeReq->partner_ids = malloc( sizeof(partners_t)
                                                            + sizeof( char * ) * 1 );

                                if( tmpdecodeReq->partner_ids != NULL ) {
                                    partnerID = wrp_strdup( StringValue );

                                    if( partnerID ) {
                                        tmpdecodeReq->partner_ids->count = 1;
                                        tmpdecodeReq->partner_ids->partner_ids[0] = partnerID;
                                    } else {
                                        tmpdecodeReq->partner_ids->count = 1;
                                        tmpdecodeReq->partner_ids->partner_ids[0] = NULL;
                                    }
                                } else {
                                    WRP_ERROR( "Memory allocation failed\n" );
                                }
                            }

                            free( NewStringVal );
                        } else {
                            WRP_ERROR( "Memory allocation failed\n" );
                        }
                    }
                    break;

                    case MSGPACK_OBJECT_BIN: {
                        if( strcmp( keyName, WRP_PAYLOAD.name ) == 0 ) {
                            binValueSize = ValueType.via.bin.size;
                            payload = calloc( 1, binValueSize + 1 );

                            if( payload != NULL ) {
                                keyValue = NULL;
                                keyValue = getKey_MsgtypeBin( ValueType, binValueSize, payload );

                                if( keyValue != NULL ) {
                                    WRP_DEBUG( "Binary payload %s\n", keyValue );
                                }

                                tmpdecodeReq->payload = keyValue;
                                tmpdecodeReq->payload_size = binValueSize;
                            } else {
                                WRP_ERROR( "Memory allocation failed\n" );
                            }
                        }
                    }
                    break;

                    case MSGPACK_OBJECT_ARRAY:

                        if( strcmp( keyName, WRP_HEADERS.name ) == 0 ) {
                            msgpack_object_array array = ValueType.via.array;
                            msgpack_object *ptr = array.ptr;
                            uint32_t cnt = 0;
                            ptr = array.ptr;
                            tmpdecodeReq->headers = malloc( sizeof(headers_t)
                                                    + sizeof( char * ) * array.size );

                            if( tmpdecodeReq->headers != NULL ) {
                                tmpdecodeReq->headers->count = array.size;

                                for( cnt = 0; cnt < array.size; cnt++, ptr++ ) {
                                    tmpdecodeReq->headers->headers[cnt] = calloc( 1, ptr->via.str.size + 1 );

                                    if( tmpdecodeReq->headers->headers[cnt] != NULL ) {
                                        memcpy( tmpdecodeReq->headers->headers[cnt], ptr->via.str.ptr, ptr->via.str.size );
                                        WRP_DEBUG( "tmpdecodeReq->headers[%d] %s\n", cnt, tmpdecodeReq->headers->headers[cnt] );
                                    }
                                }
                            }
                        } else if( strcmp( keyName, WRP_PARTNER_IDS.name ) == 0 ) {
                            msgpack_object_array array = ValueType.via.array;
                            msgpack_object *ptr = array.ptr;
                            ptr = array.ptr;
                            tmpdecodeReq->partner_ids = malloc( sizeof(partners_t)
                                                        + sizeof( char * ) * array.size );

                            if( tmpdecodeReq->partner_ids != NULL ) {
                                tmpdecodeReq->partner_ids->count = array.size;

                                for( uint32_t cnt = 0; cnt < array.size; cnt++, ptr++ ) {
                                    tmpdecodeReq->partner_ids->partner_ids[cnt] = calloc( 1, ptr->via.str.size + 1 );

                                    if( tmpdecodeReq->partner_ids->partner_ids[cnt] != NULL ) {
                                        memcpy( tmpdecodeReq->partner_ids->partner_ids[cnt], ptr->via.str.ptr,
                                                ptr->via.str.size );
                                        WRP_DEBUG( "tmpdecodeReq->partner_ids[%d] %s\n", cnt,
                                                   tmpdecodeReq->partner_ids->partner_ids[cnt] );
                                    }
                                }
                            }
                        } else {
                            WRP_ERROR( "Not Handled MSGPACK_OBJECT_ARRAY %s\n", keyName );
                        }

                        break;

                    case MSGPACK_OBJECT_MAP:
                        WRP_DEBUG( "Type of MAP\n" );
                        WRP_DEBUG( "keyName is %s\n", keyName );
                        decodeMapRequest( ValueType, decodeReq );
                        break;

                    case MSGPACK_OBJECT_NIL:

                        if( strcmp( keyName, WRP_SPANS.name ) == 0 ) {
                            WRP_DEBUG( "spans is nil\n" );
                        }

                        break;

                    default:
                        WRP_ERROR( "Unknown Data Type\n" );
                        break;
                }
            }

            p++;
            i++;
            free( keyString );
        } else {
            WRP_ERROR( "Memory allocation failed\n" );
        }
    }
}


static void decodeMapRequest( msgpack_object deserialized,
                              struct req_res_t **decodeMapReq )
{
    unsigned int i = 0;
    int n = 0, v = 0;
    int keySize = 0;
    char* keyString = NULL;
    char* keyName = NULL;
    int StringValueSize = 0;
    char* NewStringVal, *StringValue;
    char* mapName = NULL;
    char *mapValue = NULL;
    struct req_res_t *mapdecodeReq = *decodeMapReq;
    msgpack_object_kv* p = deserialized.via.map.ptr;
    WRP_DEBUG( "Map size is %d\n", deserialized.via.map.size );

    WRP_DEBUG( "mapdecodeReq->metadata->count is %d\n", deserialized.via.map.size );

    if( deserialized.via.map.size != 0 ) {
        mapdecodeReq->metadata->count = deserialized.via.map.size;
        mapdecodeReq->metadata->data_items = malloc( sizeof(struct data) *
                                             ( deserialized.via.map.size ) );

    }

    if( mapdecodeReq->metadata->data_items != NULL ) {
        while( i < deserialized.via.map.size ) {
            msgpack_object keyType = p->key;
            msgpack_object ValueType = p->val;
            keySize = keyType.via.str.size;
            keyString = malloc( keySize + 1 );

            if( keyString != NULL ) {
                keyName = NULL;
                keyName = getKey_MsgtypeStr( keyType, keySize, keyString );

                if( keyName != NULL ) {
                    mapName = wrp_strdup( keyName );

                    if( mapName ) {
                        mapdecodeReq->metadata->data_items[n].name = mapName;
                    } else {
                        mapdecodeReq->metadata->data_items[n].name = NULL;
                    }

                    n++;

                    switch( ValueType.type ) {
                        case MSGPACK_OBJECT_POSITIVE_INTEGER: {
                            WRP_DEBUG( "Map value is int %" PRId64 "\n", ValueType.via.i64 );
                            sprintf( mapdecodeReq->metadata->data_items[v].value, "%" PRId64, ValueType.via.i64 );
                            v++;
                        }
                        break;

                        case MSGPACK_OBJECT_BOOLEAN: {
                            WRP_DEBUG( "Map value boolean %d\n", ValueType.via.boolean ? true : false );
                            mapdecodeReq->metadata->data_items[v].value = ValueType.via.boolean ? "true" : "false";
                            v++;
                        }
                        break;

                        case MSGPACK_OBJECT_STR: {
                            StringValueSize = ValueType.via.str.size;
                            NewStringVal = malloc( StringValueSize + 1 );

                            if( NewStringVal != NULL ) {
                                StringValue = getKey_MsgtypeStr( ValueType, StringValueSize, NewStringVal );
                                mapValue = wrp_strdup( StringValue );
                                mapdecodeReq->metadata->data_items[v].value = mapValue;
                                v++;
                                free( NewStringVal );
                            } else {
                                WRP_ERROR( "Memory allocation failed\n" );
                            }
                        }
                        break;

                        default:
                            WRP_ERROR( "Unknown data format inside MAP\n" );
                            break;
                    }
                }

                p++;
                i++;
                free( keyString );
            } else {
                WRP_ERROR( "Memory allocation failed\n" );
            }
        }
    } else {
        WRP_ERROR( "Memory allocation failed\n" );
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

    memcpy( keyString, keyName, keySize );
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
