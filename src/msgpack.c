/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <msgpack.h>

#include "utils.h"
#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define MAKE_TOKEN(s)   { .name = s, .length = sizeof( s ) - 1 }

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

struct all_fields {
    const int64_t *msg_type;
    const int64_t *status;
    const int64_t *rdr;

    const bool *include_spans;

    const msgpack_object_str *accept;
    const msgpack_object_str *content_type;
    const msgpack_object_str *dest;
    const msgpack_object_str *path;
    const msgpack_object_str *service_name;
    const msgpack_object_str *source;
    const msgpack_object_str *url;
    const msgpack_object_str *uuid;

    const msgpack_object_bin *payload;

    const msgpack_object_array *headers;
    const msgpack_object_array *partner_ids;
    const msgpack_object_array *spans;

    const msgpack_object_map *metadata;
};

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static const struct wrp_token WRP_MSG_TYPE      = MAKE_TOKEN( "msg_type" );
static const struct wrp_token WRP_STATUS        = MAKE_TOKEN( "status" );
static const struct wrp_token WRP_SOURCE        = MAKE_TOKEN( "source" );
static const struct wrp_token WRP_DEST          = MAKE_TOKEN( "dest" );
static const struct wrp_token WRP_TRANS_ID      = MAKE_TOKEN( "transaction_uuid" );
static const struct wrp_token WRP_CONTENT_TYPE  = MAKE_TOKEN( "content_type" );
static const struct wrp_token WRP_ACCEPT        = MAKE_TOKEN( "accept" );
static const struct wrp_token WRP_HEADERS       = MAKE_TOKEN( "headers" );
static const struct wrp_token WRP_PAYLOAD       = MAKE_TOKEN( "payload" );
static const struct wrp_token WRP_SPANS         = MAKE_TOKEN( "spans" );
static const struct wrp_token WRP_INCLUDE_SPANS = MAKE_TOKEN( "include_spans" );
static const struct wrp_token WRP_SERVICE_NAME  = MAKE_TOKEN( "service_name" );
static const struct wrp_token WRP_URL           = MAKE_TOKEN( "url" );
static const struct wrp_token WRP_METADATA      = MAKE_TOKEN( "metadata" );
static const struct wrp_token WRP_RDR           = MAKE_TOKEN( "rdr" );
static const struct wrp_token WRP_PATH          = MAKE_TOKEN( "path" );
static const struct wrp_token WRP_PARTNER_IDS   = MAKE_TOKEN( "partner_ids" );

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                              Encoder Helpers                               */
/*----------------------------------------------------------------------------*/

static void enc_nstring( msgpack_packer *pk, const char *s, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, s, n );
}


static void enc_string( msgpack_packer *pk, const char *s )
{
    enc_nstring( pk, s, strlen(s) );
}


static void enc_key_int( msgpack_packer *pk, const struct wrp_token *token, int value )
{
    enc_nstring( pk, token->name, token->length );
    msgpack_pack_int( pk, value );
}


static void enc_key_string( msgpack_packer *pk, const struct wrp_token *token,
                            const char *value )
{
    if( value ) {
        enc_nstring( pk, token->name, token->length );
        enc_string( pk, value );
    }
}

static void enc_msg_type( msgpack_packer *pk, enum wrp_msg_type type )
{
    enc_key_int( pk, &WRP_MSG_TYPE, (int) type );
}


static void enc_status( msgpack_packer *pk, int status )
{
    enc_key_int( pk, &WRP_STATUS, status );
}


static void enc_source( msgpack_packer *pk, const char *source )
{
    enc_key_string( pk, &WRP_SOURCE, source );
}


static void enc_dest( msgpack_packer *pk, const char *dest )
{
    enc_key_string( pk, &WRP_DEST, dest );
}


static void enc_uuid( msgpack_packer *pk, const char *uuid )
{
    enc_key_string( pk, &WRP_TRANS_ID, uuid );
}


static void enc_content_type( msgpack_packer *pk, const char *ct )
{
    enc_key_string( pk, &WRP_CONTENT_TYPE, ct );
}


static void enc_accept( msgpack_packer *pk, const char *accept )
{
    enc_key_string( pk, &WRP_ACCEPT, accept );
}


static void enc_headers( msgpack_packer *pk, const headers_t *headers )
{
    if( headers && (0 < headers->count) ) {
        enc_nstring( pk, WRP_HEADERS.name, WRP_HEADERS.length );
        msgpack_pack_array( pk, headers->count );

        for( size_t i = 0; i < headers->count; i++ ) {
            enc_string( pk, headers->headers[i] );
        }
    }
}


static void enc_payload( msgpack_packer *pk, const void *payload, size_t len )
{
    enc_nstring( pk, WRP_PAYLOAD.name, WRP_PAYLOAD.length );
    if( !payload || !len ) {
        payload = NULL;
        len = 0;
    }

    msgpack_pack_bin( pk, len );
    msgpack_pack_bin_body( pk, payload, len );
}


static void enc_spans( msgpack_packer *pk, const struct money_trace_spans *spans )
{
    if( spans && spans->spans && (0 < spans->count) ) {
        const struct money_trace_span *span = spans->spans;

        enc_nstring( pk, WRP_SPANS.name, WRP_SPANS.length );
        msgpack_pack_array( pk, spans->count );

        for( size_t i = 0; i < spans->count; i++ ) {
            msgpack_pack_array( pk, 3 );
            enc_string( pk, span->name );
            msgpack_pack_uint64( pk, span->start );
            msgpack_pack_uint32( pk, span->duration );
            span++;
        }
    }
}


static void enc_include_spans( msgpack_packer *pk, bool inc )
{
    if( inc ) {
        enc_nstring( pk, WRP_INCLUDE_SPANS.name, WRP_INCLUDE_SPANS.length );
        msgpack_pack_true( pk );
    }
}


static void enc_service_name( msgpack_packer *pk, const char *name )
{
    enc_key_string( pk, &WRP_SERVICE_NAME, name );
}


static void enc_url( msgpack_packer *pk, const char *url )
{
    enc_key_string( pk, &WRP_URL, url );
}


static void enc_metadata( msgpack_packer *pk, const data_t *data )
{
    if( data && (0 < data->count) ) {
        enc_nstring( pk, WRP_METADATA.name, WRP_METADATA.length );

        msgpack_pack_map( pk, data->count );

        for( size_t i = 0; i < data->count; i++ ) {
            enc_string( pk, data->data_items[i].name );
            enc_string( pk, data->data_items[i].value );
        }
    }
}


static void enc_rdr( msgpack_packer *pk, int rdr )
{
    enc_key_int( pk, &WRP_RDR, rdr );
}


static void enc_path( msgpack_packer *pk, const char *path )
{
    enc_key_string( pk, &WRP_PATH, path );
}


static void enc_partner_ids( msgpack_packer *pk, const partners_t *ids )
{
    if( ids && (0 < ids->count) ) {
        enc_nstring( pk, WRP_PARTNER_IDS.name, WRP_PARTNER_IDS.length );
        msgpack_pack_array( pk, ids->count );

        for( size_t i = 0; i < ids->count; i++ ) {
            enc_string( pk, ids->partner_ids[i] );
        }
    }
}


/*----------------------------------------------------------------------------*/
/*                                  Encoders                                  */
/*----------------------------------------------------------------------------*/


static void enc_auth( msgpack_packer *pk, const struct wrp_auth_msg *a )
{
    msgpack_pack_map( pk, 2 );

    enc_msg_type( pk, WRP_MSG_TYPE__AUTH );
    enc_status( pk, a->status );
}


static void enc_req( msgpack_packer *pk, const struct wrp_req_msg *req )
{
    /* Required:
     *   msg_type 
     *   transaction_uuid
     *   source
     *   dest
     *   payload
     */
    int count = 5;

    count += (req->content_type)                          ? 1 : 0;
    count += (req->accept)                                ? 1 : 0;
    count += (req->partner_ids)                           ? 1 : 0;
    count += (req->headers)                               ? 1 : 0;
    count += (req->metadata)                              ? 1 : 0;
    count += (req->spans.spans && (0 < req->spans.count)) ? 1 : 0;
    count += (req->include_spans)                         ? 1 : 0;

    msgpack_pack_map(   pk, count );
    enc_msg_type(       pk, WRP_MSG_TYPE__REQ );
    enc_source(         pk, req->source );
    enc_dest(           pk, req->dest );
    enc_partner_ids(    pk, req->partner_ids );
    enc_headers(        pk, req->headers );
    enc_metadata(       pk, req->metadata );
    enc_uuid(           pk, req->transaction_uuid );
    enc_content_type(   pk, req->content_type );
    enc_accept(         pk, req->accept );
    enc_include_spans(  pk, req->include_spans );
    enc_spans(          pk, &req->spans );
    enc_payload(        pk, req->payload, req->payload_size );
}


static void enc_event( msgpack_packer *pk, const struct wrp_event_msg *event )
{
    /* Required:
     *   msg_type 
     *   source
     *   dest
     *   payload
     */
    int count = 4;

    count += (event->content_type) ? 1 : 0;
    count += (event->partner_ids)  ? 1 : 0;
    count += (event->headers)      ? 1 : 0;
    count += (event->metadata)     ? 1 : 0;

    msgpack_pack_map(   pk, count );
    enc_msg_type(       pk, WRP_MSG_TYPE__EVENT );
    enc_source(         pk, event->source );
    enc_dest(           pk, event->dest );
    enc_partner_ids(    pk, event->partner_ids );
    enc_headers(        pk, event->headers );
    enc_metadata(       pk, event->metadata );
    enc_content_type(   pk, event->content_type );
    enc_payload(        pk, event->payload, event->payload_size );
}


static void enc_crud( msgpack_packer *pk, const struct wrp_crud_msg *crud,
                      enum wrp_msg_type type )
{
    /* Required:
     *   msg_type 
     *   transaction_uuid
     *   source
     *   dest
     */
    int count = 4;

    count += (crud->content_type)                           ? 1 : 0;
    count += (crud->accept)                                 ? 1 : 0;
    count += (crud->partner_ids)                            ? 1 : 0;
    count += (crud->headers)                                ? 1 : 0;
    count += (crud->metadata)                               ? 1 : 0;
    count += (crud->spans.spans && (0 < crud->spans.count)) ? 1 : 0;
    count += (crud->include_spans)                          ? 1 : 0;
    count += (crud->payload && (0 < crud->payload_size))    ? 1 : 0;
    count += (0 != crud->status)                            ? 1 : 0;
    count += (0 <= crud->rdr)                               ? 1 : 0;
    count += (crud->path)                                   ? 1 : 0;

    msgpack_pack_map(   pk, count );
    enc_msg_type(       pk, type );
    enc_source(         pk, crud->source );
    enc_dest(           pk, crud->dest );
    enc_partner_ids(    pk, crud->partner_ids );
    enc_headers(        pk, crud->headers );
    enc_metadata(       pk, crud->metadata );
    enc_uuid(           pk, crud->transaction_uuid );
    enc_include_spans(  pk, crud->include_spans );
    enc_spans(          pk, &crud->spans );
    if( 0 != crud->status ) {
        enc_status(     pk, crud->status );
    }
    if( 0 <= crud->rdr ) {
        enc_rdr(        pk, crud->rdr );
    }
    enc_path(           pk, crud->path );
    enc_content_type(   pk, crud->content_type );
    enc_accept(         pk, crud->accept );

    if( crud->payload && (0 < crud->payload_size) ) {
        enc_payload( pk, crud->payload, crud->payload_size );
    }
}


static void enc_svc_reg( msgpack_packer *pk, const struct wrp_svc_registration_msg *r )
{
    msgpack_pack_map( pk, 3 );
    enc_msg_type(     pk, WRP_MSG_TYPE__SVC_REGISTRATION );
    enc_service_name( pk, r->service_name );
    enc_url(          pk, r->url );
}


static void enc_svc_alive( msgpack_packer *pk )
{
    msgpack_pack_map( pk, 1 );
    enc_msg_type(     pk, WRP_MSG_TYPE__SVC_ALIVE );
}


/*----------------------------------------------------------------------------*/
/*                              Decoder Helpers                               */
/*----------------------------------------------------------------------------*/

static bool is_key( const msgpack_object *a, const struct wrp_token *token )
{
    if( (a->via.str.size == token->length) &&
        (0 == memcmp(a->via.str.ptr, token->name, token->length)) )
    {
        return true;
    }

    return false;
}


static int mp_strdup( const msgpack_object_str *s, char **out )
{
    if( s ) {
        *out = wrp_strndup( s->ptr, (size_t) s->size );
        if( NULL == *out ) {
            return -1;
        }
    }

    return 0;
}


static int mp_bindup( const msgpack_object_bin *b, void **out, size_t *len )
{
    if( b ) {
        *out = NULL;
        *len = 0;
        if( b->size ) {
            *out = malloc( b->size );
            if( NULL == *out ) {
                return -1;
            }
            memcpy( *out, b->ptr, b->size );
            *len = b->size;
        }
    }

    return 0;
}


static int mp_partners_dup( const msgpack_object_array *a, partners_t **out )
{
    partners_t *p = NULL;

    if( !a ) {
        return 0;
    }

    p = calloc( 1, sizeof(partners_t) + (sizeof(char*) * a->size) );
    if( NULL == p ) {
        return -1;
    }
    p->count = (size_t) a->size;

    *out = p;
    for( size_t i = 0; i < p->count; i++ ) {
        if( mp_strdup(&a->ptr[i].via.str, &p->partner_ids[i]) ) {
            return -1;
        }
    }

    return 0;
}


static int mp_headers_dup( const msgpack_object_array *a, headers_t **out )
{
    headers_t *p = NULL;

    if( !a ) {
        return 0;
    }

    p = calloc( 1, sizeof(headers_t) + (sizeof(char*) * a->size) );
    if( NULL == p ) {
        return -1;
    }
    p->count = (size_t) a->size;

    *out = p;
    for( size_t i = 0; i < p->count; i++ ) {
        if( mp_strdup(&a->ptr[i].via.str, &p->headers[i]) ) {
            return -1;
        }
    }
    return 0;
}


static int mp_metadata_dup( const msgpack_object_map *m, data_t **out )
{
    data_t *p = NULL;

    if( !m ) {
        return 0;
    }

    p = calloc( 1, sizeof(data_t) );
    if( !p ) {
        return -1;
    }
    p->count = (size_t) m->size;

    p->data_items = calloc( p->count, sizeof(struct data) );
    if( !p->data_items ) {
        free( p );
        return -1;
    }

    *out = p;
    for( size_t i = 0; i < p->count; i++ ) {
        struct data *d = &p->data_items[i];

        /* copy the key */
        if( mp_strdup(&m->ptr[i].key.via.str, &d->name) ) {
            return -1;
        }

        switch( m->ptr[i].val.type ) {
            case MSGPACK_OBJECT_BOOLEAN:
                if( m->ptr[i].val.via.boolean ) {
                    d->value = wrp_strdup( "true" );
                } else {
                    d->value = wrp_strdup( "false" );
                }
                break;

            case MSGPACK_OBJECT_POSITIVE_INTEGER:
                d->value = maprintf( "%" PRIu64, m->ptr[i].val.via.u64 );
                break;

            case MSGPACK_OBJECT_NEGATIVE_INTEGER:
                d->value = maprintf( "%" PRId64, m->ptr[i].val.via.i64 );
                break;

            case MSGPACK_OBJECT_STR:
                d->value = wrp_strndup( m->ptr[i].val.via.str.ptr,
                                        (size_t) m->ptr[i].val.via.str.size );
                break;

            default:
                return -2;
        }

        if( !d->value ) {
            return -1;
        }
    }

    return 0;
}


static int mp_spans_dup( const msgpack_object_array *a, struct money_trace_spans *out )
{
    /* TODO: Add span support, though at this point, we probably want to
     *       adopt OpenTelemetry standards. */
    (void) a;
    (void) out;
    return 0;
}


/*----------------------------------------------------------------------------*/
/*                                  Decoders                                  */
/*----------------------------------------------------------------------------*/

static int dec_bools( const msgpack_object_kv *p, struct all_fields *all )
{
    if( is_key(&p->key, &WRP_INCLUDE_SPANS) ) {
        if( NULL == all->include_spans ) {
            all->include_spans = &p->val.via.boolean;
            return 0;
        }
    } else {
        return 0;
    }

    return -1;
}


static int dec_int( const msgpack_object_kv *p, struct all_fields *all )
{
    /* TODO: Figure out i64 vs u64 ... */
    struct list {
        const struct wrp_token *token;
        const int64_t **dest;
    } mapping[] = {
        { &WRP_MSG_TYPE, &all->msg_type },
        { &WRP_STATUS,   &all->status },
        { &WRP_RDR,      &all->rdr },
    };

    for( size_t i = 0; i < sizeof(mapping) / sizeof(struct list); i++ ) {
        if( is_key(&p->key, mapping[i].token) ) {
            if( NULL != *mapping[i].dest ) {
                return -1;
            }

            *mapping[i].dest = &p->val.via.i64;
            return 0;
        }
    }

    return 0;
}


static int dec_str( const msgpack_object_kv *p, struct all_fields *all )
{
    struct list {
        const struct wrp_token *token;
        const msgpack_object_str **dest;
    } mapping[] = {
        { &WRP_SOURCE,       &all->source },
        { &WRP_DEST,         &all->dest },
        { &WRP_TRANS_ID,     &all->uuid },
        { &WRP_SERVICE_NAME, &all->service_name },
        { &WRP_URL,          &all->url },
        { &WRP_PATH,         &all->path },
        { &WRP_CONTENT_TYPE, &all->content_type },
        { &WRP_ACCEPT,       &all->accept },
    };

    for( size_t i = 0; i < sizeof(mapping) / sizeof(struct list); i++ ) {
        if( is_key(&p->key, mapping[i].token) ) {
            if( NULL != *mapping[i].dest ) {
                return -1;
            }

            *mapping[i].dest = &p->val.via.str;
            return 0;
        }
    }

    return 0;
}


static int dec_array( const msgpack_object_kv *p, struct all_fields *all )
{
    struct list {
        const struct wrp_token *token;
        const msgpack_object_array **dest;
        msgpack_object_type type;
    } mapping[] = {
        { &WRP_HEADERS,     &all->headers,     MSGPACK_OBJECT_STR },
        { &WRP_PARTNER_IDS, &all->partner_ids, MSGPACK_OBJECT_STR },
        { &WRP_SPANS,       &all->spans,       MSGPACK_OBJECT_ARRAY },
    };

    for( size_t i = 0; i < sizeof(mapping) / sizeof(struct list); i++ ) {
        if( is_key(&p->key, mapping[i].token) ) {
            if( NULL != *mapping[i].dest ) {
                return -1;
            }

            /* Validate the types are all the same. */
            for( uint32_t j = 0; j < p->val.via.array.size; j++ ) {
                if( mapping[i].type != p->val.via.array.ptr[j].type ) {
                    return -1;
                }
            }

            *mapping[i].dest = &p->val.via.array;
            return 0;
        }
    }

    return 0;
}


static int dec_map( const msgpack_object_kv *p, struct all_fields *all )
{
    if( is_key(&p->key, &WRP_METADATA) ) {
        if( NULL == all->metadata ) {
            /* Validate the keys are all strings. */
            for( uint32_t i = 0; i < p->val.via.map.size; i++ ) {
                if( MSGPACK_OBJECT_STR != p->val.via.map.ptr[i].key.type ) {
                    return -1;
                }
            }

            all->metadata = &p->val.via.map;
            return 0;
        }
    } else {
        return 0;
    }

    return -1;
}


static int dec_bin( const msgpack_object_kv *p, struct all_fields *all )
{
    if( is_key(&p->key, &WRP_PAYLOAD) ) {
        if( NULL == all->payload ) {
            all->payload = &p->val.via.bin;
            return 0;
        }
    } else {
        return 0;
    }

    return -1;
}


static int dec_map_all_possible( const msgpack_object *obj, struct all_fields *all )
{
    const msgpack_object_kv *p = NULL;
    int rv = 0;

    memset( all, 0, sizeof(struct all_fields) );
    if( (MSGPACK_OBJECT_MAP != obj->type) || (0 == obj->via.map.size) ) {
        return -1;
    }

    p = obj->via.map.ptr;
    for( uint32_t i = 0; i < obj->via.map.size; i++, p++ ) {
        if( MSGPACK_OBJECT_STR == p->key.type ) {
            switch( p->val.type ) {
                case MSGPACK_OBJECT_BOOLEAN:
                    rv |= dec_bools( p, all );
                    break;

                case MSGPACK_OBJECT_POSITIVE_INTEGER:
                case MSGPACK_OBJECT_NEGATIVE_INTEGER:
                    rv |= dec_int( p, all );
                    break;

                case MSGPACK_OBJECT_STR:
                    rv |= dec_str( p, all );
                    break;

                case MSGPACK_OBJECT_ARRAY:
                    rv |= dec_array( p, all );
                    break;

                case MSGPACK_OBJECT_MAP:
                    rv |= dec_map( p, all );
                    break;

                case MSGPACK_OBJECT_BIN:
                    rv |= dec_bin( p, all );
                    break;

                default:
                    break;
            }
        }

        if( 0 != rv ) {
            return -1;
        }
    }

    return 0;
}


static int dec_trans_auth( const struct all_fields *all, wrp_msg_t *p )
{
    if( !all->status ) {
        return -1;
    }

    if( all->status && (INT_MAX < *all->status) ) {
        return -1;
    }

    p->u.auth.status = (int) *all->status;
    return 0;
}


static int dec_trans_req( const struct all_fields *all, wrp_msg_t *p )
{
    struct wrp_req_msg *r = &p->u.req;

    if( !all->uuid || !all->source || !all->dest || !all->payload ) {
        return -1;
    }

    r->include_spans = (all->include_spans) ? *all->include_spans : 0;

    if(    (0 == mp_strdup(all->uuid, &r->transaction_uuid))
        && (0 == mp_strdup(all->content_type, &r->content_type))
        && (0 == mp_strdup(all->accept, &r->accept))
        && (0 == mp_strdup(all->source, &r->source))
        && (0 == mp_strdup(all->dest, &r->dest))
        && (0 == mp_partners_dup(all->partner_ids, &r->partner_ids))
        && (0 == mp_headers_dup(all->headers, &r->headers))
        && (0 == mp_bindup(all->payload, &r->payload, &r->payload_size))
        && (0 == mp_metadata_dup(all->metadata, &r->metadata))
        && (0 == mp_spans_dup(all->spans, &r->spans)) )
    {
        return 0;
    }

    return -1;
}


static int dec_trans_event( const struct all_fields *all, wrp_msg_t *p )
{
    struct wrp_event_msg *e = &p->u.event;

    if( !all->source || !all->dest || !all->payload ) {
        return -1;
    }

    if(    (0 == mp_strdup(all->content_type, &e->content_type))
        && (0 == mp_strdup(all->source, &e->source))
        && (0 == mp_strdup(all->dest, &e->dest))
        && (0 == mp_partners_dup(all->partner_ids, &e->partner_ids))
        && (0 == mp_headers_dup(all->headers, &e->headers))
        && (0 == mp_metadata_dup(all->metadata, &e->metadata))
        && (0 == mp_bindup(all->payload, &e->payload, &e->payload_size)) )
    {
        return 0;
    }

    return -1;
}


static int dec_trans_crud( const struct all_fields *all, wrp_msg_t *p )
{
    struct wrp_crud_msg *c = &p->u.crud;

    if( !all->uuid || !all->source || !all->dest ) {
        return -1;
    }

    if( all->status && (INT_MAX < *all->status) ) {
        return -1;
    }
    if( all->rdr && (INT_MAX < *all->rdr) ) {
        return -1;
    }

    c->include_spans = (all->include_spans) ? *all->include_spans : 0;
    c->status = (all->status) ? (int) *all->status : 0;
    c->rdr = (all->rdr) ? (int) *all->rdr : 0;

    if(    (0 == mp_strdup(all->uuid, &c->transaction_uuid))
        && (0 == mp_strdup(all->content_type, &c->content_type))
        && (0 == mp_strdup(all->accept, &c->accept))
        && (0 == mp_strdup(all->source, &c->source))
        && (0 == mp_strdup(all->dest, &c->dest))
        && (0 == mp_strdup(all->path, &c->path))
        && (0 == mp_partners_dup(all->partner_ids, &c->partner_ids))
        && (0 == mp_headers_dup(all->headers, &c->headers))
        && (0 == mp_bindup(all->payload, &c->payload, &c->payload_size))
        && (0 == mp_metadata_dup(all->metadata, &c->metadata))
        && (0 == mp_spans_dup(all->spans, &c->spans)) )
    {
        return 0;
    }

    return -1;
}


static int dec_trans_reg( const struct all_fields *all, wrp_msg_t *p )
{
    struct wrp_svc_registration_msg *r = &p->u.reg;

    if( !all->service_name || !all->url ) {
        return -1;
    }

    if(    (0 == mp_strdup(all->url, &r->url))
        && (0 == mp_strdup(all->service_name, &r->service_name)) )
    {
        return 0;
    }

    return -1;
}


static int dec_translate( struct all_fields *all, wrp_msg_t *p )
{
    if( !all->msg_type ) {
        return -1;
    }

    p->msg_type = (enum wrp_msg_type) *all->msg_type;

    switch( p->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            return dec_trans_auth( all, p );

        case WRP_MSG_TYPE__REQ:
            return dec_trans_req( all, p );

        case WRP_MSG_TYPE__EVENT:
            return dec_trans_event( all, p );

        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            return dec_trans_crud( all, p );

        case WRP_MSG_TYPE__SVC_REGISTRATION:
            return dec_trans_reg( all, p );

        case WRP_MSG_TYPE__SVC_ALIVE:
            return 0;

        default:
            break;
    }

    return -1;
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

ssize_t wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes )
{
    ssize_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;

    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    switch( msg->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            enc_auth( &pk, &msg->u.auth );
            break;

        case WRP_MSG_TYPE__REQ:
            enc_req( &pk, &msg->u.req );
            break;

        case WRP_MSG_TYPE__EVENT:
            enc_event( &pk, &msg->u.event );
            break;

        case WRP_MSG_TYPE__SVC_REGISTRATION:
            enc_svc_reg( &pk, &msg->u.reg );
            break;

        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            enc_crud( &pk, &msg->u.crud, msg->msg_type );
            break;

        case WRP_MSG_TYPE__SVC_ALIVE:
            enc_svc_alive( &pk );
            break;

        default:
            break;
    }

    if( sbuf.data ) {
        *bytes = calloc( sbuf.size, sizeof(char) );

        if( NULL != *bytes ) {
            memcpy( *bytes, sbuf.data, sbuf.size );
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
 *  @param msg [in] data the data_t structure to pack/encode
 *  @param msg [out] the encoded output
 *  @return encoded buffer size or less than 1 in failure case
 */
ssize_t wrp_pack_metadata( const data_t *data, void **out )
{
    ssize_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;

    if( NULL == data || NULL == out ) {
        return -1;
    }

    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    enc_metadata( &pk, data );

    if( sbuf.data ) {
        *out = calloc( sbuf.size, sizeof(char) );

        if( NULL != *out ) {
            memcpy( *out, sbuf.data, sbuf.size );
            rv = sbuf.size;
        }
    }

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}


ssize_t wrp_bytes_to_struct( const void *bytes, size_t length,
                                      wrp_msg_t **msg_ptr )
{
    msgpack_zone mempool;
    msgpack_object obj;
    msgpack_unpack_return unpack_ret;
    ssize_t rv = -1;

    *msg_ptr = calloc( 1, sizeof(wrp_msg_t) );
    if( NULL == *msg_ptr ) {
        return -1;
    }

    msgpack_zone_init( &mempool, 4096 );
    unpack_ret = msgpack_unpack( bytes, length, NULL, &mempool, &obj );
    if( MSGPACK_UNPACK_SUCCESS == unpack_ret ) {
        struct all_fields all;

        if( (0 == dec_map_all_possible(&obj, &all)) &&
            (0 == dec_translate(&all, *msg_ptr)) )
        {
            rv = length;
        }
    }

    msgpack_zone_destroy( &mempool );

    if( -1 == rv ) {
        wrp_free_struct( *msg_ptr );
        *msg_ptr = NULL;
    }
    return rv;
}
