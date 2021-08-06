/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "internal.h"
#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static void get_msg_type(mpack_node_t root, enum wrp_msg_type *t)
{
    mpack_node_t val;
    uint8_t type;

    val = mpack_node_map_str(root, WRP_MSG_TYPE.s, WRP_MSG_TYPE.len);

    type = mpack_node_u8(val);
    if (mpack_ok != mpack_node_error(val)) {
        mpack_node_flag_error(root, mpack_error_data);
    } else {
        *t = (enum wrp_msg_type)type;
    }
}


static bool is_valid_node(mpack_node_t node)
{
    if (mpack_node_is_missing(node) || mpack_node_is_nil(node)) {
        return false;
    }

    return true;
}


static mpack_node_t get_node(mpack_node_t root, int flags, const struct wrp_token *token)
{
    if (OPTIONAL == flags) {
        return mpack_node_map_str_optional(root, token->s, token->len);
    }

    return mpack_node_map_str(root, token->s, token->len);
}


static void dec_str__(mpack_node_t root, int flags, const struct wrp_token *token,
                      struct wrp_string *s)
{
    mpack_node_t val;

    val = get_node(root, flags, token);
    if (is_valid_node(val)) {
        s->s = mpack_node_str(val);
        s->len = mpack_node_strlen(val);
    }
}


static void dec_int__(mpack_node_t root, int flags, const struct wrp_token *token,
                      struct wrp_int *i)
{
    mpack_node_t val;

    i->valid = false;
    i->n = 0;
    val = get_node(root, flags, token);
    if (is_valid_node(val)) {
        i->valid = true;
        i->n = mpack_node_int(val);
    }
}


static void dec_blob_(mpack_node_t root, int flags, const struct wrp_token *token,
                      struct wrp_blob *blob)
{
    mpack_node_t val;

    val = get_node(root, flags, token);
    if (is_valid_node(val)) {
        blob->data = (const uint8_t *)mpack_node_bin_data(val);
        blob->len = mpack_node_bin_size(val);
    }
}


static void dec_slist(mpack_node_t root, int flags, const struct wrp_token *token,
                      struct wrp_string_list *l, void **free_link)
{
    mpack_node_t list;

    list = get_node(root, flags, token);
    if (false == is_valid_node(list)) {
        return;
    }

    l->count = mpack_node_array_length(list);
    if (l->count) {
        l->list = calloc(l->count, sizeof(struct wrp_string));
        if (!l->list) {
            mpack_node_flag_error(list, mpack_error_memory);
            return;
        }

        /* Make freeing this easier later. */
        *free_link = l->list;

        for (size_t i = 0; i < l->count; i++) {
            mpack_node_t val;

            val = mpack_node_array_at(list, i);
            l->list[i].s = mpack_node_str(val);
            l->list[i].len = mpack_node_strlen(val);
        }
    }
}


static void dec_nvpl_(mpack_node_t root, int flags, const struct wrp_token *token,
                      struct wrp_nvp_list *l, void **free_link)
{
    mpack_node_t map;

    map = get_node(root, flags, token);
    if (false == is_valid_node(map)) {
        return;
    }

    l->count = mpack_node_map_count(map);
    if (l->count) {
        l->list = calloc(l->count, sizeof(struct wrp_nvp));
        if (!l->list) {
            mpack_node_flag_error(map, mpack_error_memory);
            return;
        }

        /* Make freeing this easier later. */
        *free_link = l->list;

        for (size_t i = 0; i < l->count; i++) {
            mpack_node_t n;
            mpack_node_t v;

            n = mpack_node_map_key_at(map, i);
            v = mpack_node_map_value_at(map, i);
            l->list[i].name.s = mpack_node_str(n);
            l->list[i].name.len = mpack_node_strlen(n);
            l->list[i].value.s = NULL;
            l->list[i].value.len = 0;
            if (!mpack_node_is_nil(v)) {
                l->list[i].value.len = mpack_node_strlen(v);
                if (l->list[i].value.len) {
                    l->list[i].value.s = mpack_node_str(v);
                }
            }
        }
    }
}


static void decode_root(struct wrp_internal *p)
{
    mpack_node_t root;
    mpack_error_t err;

    root = mpack_tree_root(&p->tree);
    get_msg_type(root, &p->msg.msg_type);
    err = mpack_tree_error(&p->tree);
    if (err != mpack_ok) {
        return;
    }

    switch (p->msg.msg_type) {
    case WRP_MSG_TYPE__AUTH:
        dec_int__(root, REQUIRED, &WRP_STATUS__, &p->msg.u.auth.status);
        break;

    case WRP_MSG_TYPE__REQ:
        dec_str__(root, REQUIRED, &WRP_SOURCE__, &p->msg.u.req.source);
        dec_str__(root, REQUIRED, &WRP_DEST____, &p->msg.u.req.dest);
        dec_str__(root, REQUIRED, &WRP_TRANS_ID, &p->msg.u.req.trans_id);
        dec_str__(root, OPTIONAL, &WRP_CT______, &p->msg.u.req.content_type);
        dec_str__(root, OPTIONAL, &WRP_ACCEPT__, &p->msg.u.req.accept);
        dec_int__(root, OPTIONAL, &WRP_RDR_____, &p->msg.u.req.rdr);
        dec_int__(root, OPTIONAL, &WRP_STATUS__, &p->msg.u.req.status);
        dec_blob_(root, OPTIONAL, &WRP_PAYLOAD_, &p->msg.u.req.payload);
        dec_slist(root, OPTIONAL, &WRP_PARTNERS, &p->msg.u.req.partner_ids, &p->partner_ids);
        dec_nvpl_(root, OPTIONAL, &WRP_METADATA, &p->msg.u.req.metadata, &p->metadata);
        dec_slist(root, OPTIONAL, &WRP_HEADERS_, &p->msg.u.req.headers, &p->headers);
        dec_str__(root, OPTIONAL, &WRP_MSG_ID__, &p->msg.u.req.msg_id);
        dec_str__(root, OPTIONAL, &WRP_SESS_ID_, &p->msg.u.req.session_id);
        break;

    case WRP_MSG_TYPE__EVENT:
        dec_str__(root, REQUIRED, &WRP_SOURCE__, &p->msg.u.event.source);
        dec_str__(root, REQUIRED, &WRP_DEST____, &p->msg.u.event.dest);
        dec_str__(root, OPTIONAL, &WRP_CT______, &p->msg.u.event.content_type);
        dec_blob_(root, OPTIONAL, &WRP_PAYLOAD_, &p->msg.u.event.payload);
        dec_slist(root, OPTIONAL, &WRP_PARTNERS, &p->msg.u.event.partner_ids, &p->partner_ids);
        dec_nvpl_(root, OPTIONAL, &WRP_METADATA, &p->msg.u.event.metadata, &p->metadata);
        dec_slist(root, OPTIONAL, &WRP_HEADERS_, &p->msg.u.event.headers, &p->headers);
        dec_str__(root, OPTIONAL, &WRP_MSG_ID__, &p->msg.u.event.msg_id);
        dec_str__(root, OPTIONAL, &WRP_SESS_ID_, &p->msg.u.event.session_id);
        break;

    case WRP_MSG_TYPE__CREATE:
    case WRP_MSG_TYPE__RETRIEVE:
    case WRP_MSG_TYPE__UPDATE:
    case WRP_MSG_TYPE__DELETE:
        dec_str__(root, REQUIRED, &WRP_SOURCE__, &p->msg.u.crud.source);
        dec_str__(root, REQUIRED, &WRP_DEST____, &p->msg.u.crud.dest);
        dec_str__(root, REQUIRED, &WRP_TRANS_ID, &p->msg.u.crud.trans_id);
        dec_str__(root, OPTIONAL, &WRP_CT______, &p->msg.u.crud.content_type);
        dec_str__(root, OPTIONAL, &WRP_ACCEPT__, &p->msg.u.crud.accept);
        dec_str__(root, OPTIONAL, &WRP_PATH____, &p->msg.u.crud.path);
        dec_int__(root, OPTIONAL, &WRP_RDR_____, &p->msg.u.crud.rdr);
        dec_int__(root, OPTIONAL, &WRP_STATUS__, &p->msg.u.crud.status);
        dec_blob_(root, OPTIONAL, &WRP_PAYLOAD_, &p->msg.u.crud.payload);
        dec_slist(root, OPTIONAL, &WRP_PARTNERS, &p->msg.u.crud.partner_ids, &p->partner_ids);
        dec_nvpl_(root, OPTIONAL, &WRP_METADATA, &p->msg.u.crud.metadata, &p->metadata);
        dec_slist(root, OPTIONAL, &WRP_HEADERS_, &p->msg.u.crud.headers, &p->headers);
        dec_str__(root, OPTIONAL, &WRP_MSG_ID__, &p->msg.u.crud.msg_id);
        dec_str__(root, OPTIONAL, &WRP_SESS_ID_, &p->msg.u.crud.session_id);
        break;

    case WRP_MSG_TYPE__SVC_REG:
        dec_str__(root, REQUIRED, &WRP_SN______, &p->msg.u.reg.service_name);
        dec_str__(root, REQUIRED, &WRP_URL_____, &p->msg.u.reg.url);
        break;

    case WRP_MSG_TYPE__SVC_ALIVE:
        break;

    default:
        mpack_node_flag_error(root, mpack_error_data);
    }
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/


WRPcode wrp_from_msgpack(const void *data, size_t len, wrp_msg_t **msg)
{
    struct wrp_internal *p;
    mpack_error_t err;
    WRPcode rv = WRPE_OK;

    if (!data || !len || !(msg)) {
        return WRPE_INVALID_ARGS;
    }

    p = calloc(1, sizeof(struct wrp_internal));
    if (!p) {
        return WRPE_OUT_OF_MEMORY;
    }

    p->sig = INTERNAL_SIGNATURE;

    mpack_tree_init_data(&p->tree, data, len);
    mpack_tree_parse(&p->tree);
    decode_root(p);
    err = mpack_tree_error(&p->tree);

    rv = map_mpack_err(err);
    if (WRPE_OK != rv) {
        mpack_tree_destroy(&p->tree);
        free(p);
    } else {
        p->msg.__internal_only = (void *)p;
        *msg = &p->msg;
    }

    return rv;
}


WRPcode wrp_destroy(wrp_msg_t *msg)
{
    struct wrp_internal *p = NULL;

    if (!msg) {
        return WRPE_OK;
    }

    p = (struct wrp_internal *)msg->__internal_only;
    if (!p || (INTERNAL_SIGNATURE != p->sig)) {
        return WRPE_NOT_FROM_WRPC;
    }

    mpack_tree_destroy(&p->tree);

    if (p->partner_ids) {
        free(p->partner_ids);
    }
    if (p->metadata) {
        free(p->metadata);
    }
    if (p->headers) {
        free(p->headers);
    }

    free(p);

    return WRPE_OK;
}
