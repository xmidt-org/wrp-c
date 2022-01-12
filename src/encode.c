/* SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "internal.h"
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
/* none */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static void enc_str__(mpack_writer_t *w, int flags, const struct wrp_token *token,
                      const struct wrp_string *s)
{
    bool val_present = ((0 < s->len) && (NULL != s->s)) ? true : false;

    if (val_present || (REQUIRED == flags)) {
        mpack_write_str(w, token->s, (uint32_t) token->len);
    }

    if (val_present) {
        mpack_write_str(w, s->s, (uint32_t) s->len);
    } else if (REQUIRED) {
        mpack_write_str(w, NULL, 0);
    }
}


static void enc_mtype(mpack_writer_t *w, enum wrp_msg_type i)
{
    mpack_write_str(w, WRP_MSG_TYPE.s, (uint32_t) WRP_MSG_TYPE.len);
    mpack_write_int(w, i);
}


static void enc_int__(mpack_writer_t *w, int flags, const struct wrp_token *token,
                      const struct wrp_int *i)
{
    if (i->num || (REQUIRED == flags)) {
        mpack_write_str(w, token->s, (uint32_t) token->len);
    }

    if (i->num) {
        mpack_write_int(w, *i->num);
    } else if (REQUIRED) {
        mpack_write_nil(w);
    }
}


static void enc_blob_(mpack_writer_t *w, int flags, const struct wrp_token *token,
                      const struct wrp_blob *blob)
{
    bool val_present = ((0 < blob->len) && (NULL != blob->data)) ? true : false;

    if (val_present || (REQUIRED == flags)) {
        mpack_write_str(w, token->s, (uint32_t) token->len);
    }

    if (val_present) {
        mpack_write_bin(w, (const char *) blob->data, (uint32_t) blob->len);
    } else if (REQUIRED) {
        mpack_write_bin(w, NULL, 0);
    }
}


static void enc_slist(mpack_writer_t *w, int flags, const struct wrp_token *token,
                      const struct wrp_string_list *l)
{
    bool val_present = ((0 < l->count) && (NULL != l->list)) ? true : false;

    if (val_present || (REQUIRED == flags)) {
        mpack_write_str(w, token->s, (uint32_t) token->len);
    }

    if (val_present) {
        mpack_start_array(w, (uint32_t) l->count);
        for (size_t i = 0; i < l->count; i++) {
            mpack_write_str(w, l->list[i].s, (uint32_t) l->list[i].len);
        }
        mpack_finish_array(w);
    } else if (REQUIRED) {
        mpack_start_array(w, 0);
        mpack_finish_array(w);
    }
}


static void enc_nvpl_(mpack_writer_t *w, int flags, const struct wrp_token *token,
                      const struct wrp_nvp_list *l)
{
    bool val_present = ((0 < l->count) && (NULL != l->list)) ? true : false;

    if (val_present || (REQUIRED == flags)) {
        mpack_write_str(w, token->s, (uint32_t) token->len);
    }

    if (val_present) {
        mpack_start_map(w, (uint32_t) l->count);
        for (size_t i = 0; i < l->count; i++) {
            mpack_write_str(w, l->list[i].name.s, (uint32_t) l->list[i].name.len);
            mpack_write_str(w, l->list[i].value.s, (uint32_t) l->list[i].value.len);
        }
        mpack_finish_map(w);
    } else if (REQUIRED) {
        mpack_start_map(w, 0);
        mpack_finish_map(w);
    }
}


static void enc_auth(mpack_writer_t *w, const struct wrp_auth_msg *a)
{
    mpack_start_map(w, 2);
    enc_mtype(w, WRP_MSG_TYPE__AUTH);
    enc_int__(w, REQUIRED, &WRP_STATUS__, &a->status);
    mpack_finish_map(w);
}


static void enc_req(mpack_writer_t *w, const struct wrp_req_msg *req)
{
    /* Required:
     *   msg_type
     *   transaction_uuid
     *   source
     *   dest
     *   payload
     */
    int count = 5;

    count += (req->accept.len) ? 1 : 0;
    count += (req->content_type.len) ? 1 : 0;
    count += (req->headers.count) ? 1 : 0;
    count += (req->metadata.count) ? 1 : 0;
    count += (req->msg_id.len) ? 1 : 0;
    count += (req->partner_ids.count) ? 1 : 0;
    count += (req->rdr.num) ? 1 : 0;
    count += (req->session_id.len) ? 1 : 0;
    count += (req->status.num) ? 1 : 0;

    mpack_start_map(w, count);
    enc_mtype(w, WRP_MSG_TYPE__REQ);
    enc_str__(w, REQUIRED, &WRP_DEST____, &req->dest);
    enc_blob_(w, REQUIRED, &WRP_PAYLOAD_, &req->payload);
    enc_str__(w, REQUIRED, &WRP_SOURCE__, &req->source);
    enc_str__(w, REQUIRED, &WRP_TRANS_ID, &req->trans_id);

    enc_str__(w, OPTIONAL, &WRP_ACCEPT__, &req->accept);
    enc_str__(w, OPTIONAL, &WRP_CT______, &req->content_type);
    enc_slist(w, OPTIONAL, &WRP_HEADERS_, &req->headers);
    enc_nvpl_(w, OPTIONAL, &WRP_METADATA, &req->metadata);
    enc_str__(w, OPTIONAL, &WRP_MSG_ID__, &req->msg_id);
    enc_slist(w, OPTIONAL, &WRP_PARTNERS, &req->partner_ids);
    enc_int__(w, OPTIONAL, &WRP_RDR_____, &req->rdr);
    enc_str__(w, OPTIONAL, &WRP_SESS_ID_, &req->session_id);
    enc_int__(w, OPTIONAL, &WRP_STATUS__, &req->status);
    mpack_finish_map(w);
}


static void enc_event(mpack_writer_t *w, const struct wrp_event_msg *event)
{
    /* Required:
     *   msg_type
     *   source
     *   dest
     */
    int count = 3;

    count += (event->content_type.len) ? 1 : 0;
    count += (event->payload.len) ? 1 : 0;
    count += (event->partner_ids.count) ? 1 : 0;
    count += (event->metadata.count) ? 1 : 0;
    count += (event->headers.count) ? 1 : 0;
    count += (event->msg_id.len) ? 1 : 0;
    count += (event->session_id.len) ? 1 : 0;

    mpack_start_map(w, count);
    enc_mtype(w, WRP_MSG_TYPE__EVENT);
    enc_str__(w, REQUIRED, &WRP_DEST____, &event->dest);
    enc_str__(w, REQUIRED, &WRP_SOURCE__, &event->source);
    enc_str__(w, OPTIONAL, &WRP_CT______, &event->content_type);
    enc_slist(w, OPTIONAL, &WRP_HEADERS_, &event->headers);
    enc_nvpl_(w, OPTIONAL, &WRP_METADATA, &event->metadata);
    enc_str__(w, OPTIONAL, &WRP_MSG_ID__, &event->msg_id);
    enc_slist(w, OPTIONAL, &WRP_PARTNERS, &event->partner_ids);
    enc_blob_(w, OPTIONAL, &WRP_PAYLOAD_, &event->payload);
    enc_str__(w, OPTIONAL, &WRP_SESS_ID_, &event->session_id);
    mpack_finish_map(w);
}


static void enc_crud(mpack_writer_t *w, const struct wrp_crud_msg *crud,
                     enum wrp_msg_type msg_type)
{
    /* Required:
     *   msg_type
     *   transaction_uuid
     *   source
     *   dest
     */
    int count = 4;

    count += (crud->accept.len) ? 1 : 0;
    count += (crud->content_type.len) ? 1 : 0;
    count += (crud->headers.count) ? 1 : 0;
    count += (crud->metadata.count) ? 1 : 0;
    count += (crud->msg_id.len) ? 1 : 0;
    count += (crud->partner_ids.count) ? 1 : 0;
    count += (crud->path.len) ? 1 : 0;
    count += (crud->payload.len) ? 1 : 0;
    count += (crud->rdr.num) ? 1 : 0;
    count += (crud->session_id.len) ? 1 : 0;
    count += (crud->status.num) ? 1 : 0;

    mpack_start_map(w, count);
    enc_mtype(w, msg_type);
    enc_str__(w, REQUIRED, &WRP_DEST____, &crud->dest);
    enc_str__(w, REQUIRED, &WRP_SOURCE__, &crud->source);
    enc_str__(w, REQUIRED, &WRP_TRANS_ID, &crud->trans_id);
    enc_str__(w, OPTIONAL, &WRP_ACCEPT__, &crud->accept);
    enc_str__(w, OPTIONAL, &WRP_CT______, &crud->content_type);
    enc_slist(w, OPTIONAL, &WRP_HEADERS_, &crud->headers);
    enc_nvpl_(w, OPTIONAL, &WRP_METADATA, &crud->metadata);
    enc_str__(w, OPTIONAL, &WRP_MSG_ID__, &crud->msg_id);
    enc_slist(w, OPTIONAL, &WRP_PARTNERS, &crud->partner_ids);
    enc_str__(w, OPTIONAL, &WRP_PATH____, &crud->path);
    enc_blob_(w, OPTIONAL, &WRP_PAYLOAD_, &crud->payload);
    enc_int__(w, OPTIONAL, &WRP_RDR_____, &crud->rdr);
    enc_str__(w, OPTIONAL, &WRP_SESS_ID_, &crud->session_id);
    enc_int__(w, OPTIONAL, &WRP_STATUS__, &crud->status);
    mpack_finish_map(w);
}


static void enc_svc_reg(mpack_writer_t *w, const struct wrp_svc_reg_msg *r)
{
    mpack_start_map(w, 3);
    enc_mtype(w, WRP_MSG_TYPE__SVC_REG);
    enc_str__(w, REQUIRED, &WRP_SN______, &r->service_name);
    enc_str__(w, REQUIRED, &WRP_URL_____, &r->url);
    mpack_finish_map(w);
}


static void enc_svc_alive(mpack_writer_t *w)
{
    mpack_start_map(w, 1);
    enc_mtype(w, WRP_MSG_TYPE__SVC_ALIVE);
    mpack_finish_map(w);
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
WRPcode wrp_to_msgpack(const wrp_msg_t *msg, uint8_t **buf, size_t *len)
{
    mpack_writer_t writer;
    WRPcode rv = WRPE_OK;

    if (!msg || !buf || !len) {
        return WRPE_INVALID_ARGS;
    }

    if (*buf) {
        mpack_writer_init(&writer, (char *) *buf, *len);
    } else {
        mpack_writer_init_growable(&writer, (char **) buf, len);
    }

    switch (msg->msg_type) {
        case WRP_MSG_TYPE__AUTH:
            enc_auth(&writer, &msg->u.auth);
            break;

        case WRP_MSG_TYPE__REQ:
            enc_req(&writer, &msg->u.req);
            break;

        case WRP_MSG_TYPE__EVENT:
            enc_event(&writer, &msg->u.event);
            break;

        case WRP_MSG_TYPE__SVC_REG:
            enc_svc_reg(&writer, &msg->u.reg);
            break;

        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETRIEVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            enc_crud(&writer, &msg->u.crud, msg->msg_type);
            break;

        case WRP_MSG_TYPE__SVC_ALIVE:
            enc_svc_alive(&writer);
            break;

        default:
            rv = WRPE_NOT_A_WRP_MSG;
            break;
    }

    if (WRPE_OK == rv) {
        mpack_error_t err;

        err = mpack_writer_error(&writer);

        rv = map_mpack_err(err);
    }

    if (WRPE_OK == rv) {
        /* Set the buffer used to exactly that size vs. what might have
         * been allocated extra. */
        *len = mpack_writer_buffer_used(&writer);
    }

    mpack_writer_destroy(&writer);
    return rv;
}
