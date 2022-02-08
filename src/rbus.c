/* SPDX-FileCopyrightText: 2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <cutils/strings.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/wrp-c/rbus.h"
#include "constants.h"

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
static WRPcode add_int___(rbusObject_t obj, const char *name, const struct wrp_int *i, WRPcode *err)
{
    if (WRPE_OK != *err) {
        return *err;
    }

    if (i) {
        rbusValue_t value = rbusValue_FromInt32((int32_t) *i->num);

        if (value) {
            rbusObject_SetValue(obj, name, value);
        } else {
            *err = WRPE_OUT_OF_MEMORY;
        }
    }

    return *err;
}


static WRPcode add_bytes_(rbusObject_t obj, const char *name, const struct wrp_blob *b, WRPcode *err)
{
    if (WRPE_OK != *err) {
        return *err;
    }

    if (b && (0 < b->len)) {
        rbusValue_t value = rbusValue_FromBytes(b->data, (int) b->len);
        if (value) {
            rbusObject_SetValue(obj, name, value);
        } else {
            *err = WRPE_OUT_OF_MEMORY;
        }
    }

    return *err;
}


static WRPcode add_string(rbusObject_t obj, const char *name, const struct wrp_string *s, WRPcode *err)
{
    if (WRPE_OK != *err) {
        return *err;
    }

    if (s && (0 < s->len)) {
        char *str = cu_strndup(s->s, s->len);

        if (str) {
            rbusValue_t value = rbusValue_FromString(str);

            if (value) {
                rbusObject_SetValue(obj, name, value);
            } else {
                *err = WRPE_OUT_OF_MEMORY;
            }

            free(str);
        } else {
            *err = WRPE_OUT_OF_MEMORY;
        }
    }

    return *err;
}


static WRPcode add_msg_type(rbusObject_t obj, const char *type, WRPcode *err)
{
    if (WRPE_OK != *err) {
        return *err;
    }

    rbusValue_t value = rbusValue_FromString(type);

    if (value) {
        rbusObject_SetValue(obj, WRP_MSG_TYPE.s, value);
    } else {
        *err = WRPE_OUT_OF_MEMORY;
    }

    return *err;
}


static WRPcode handle_auth(rbusObject_t obj, const struct wrp_auth_msg *auth)
{
    WRPcode err = WRPE_OK;

    return add_int___(obj, WRP_STATUS__.s, &auth->status, &err);
}


static WRPcode handle_req(rbusObject_t obj, const struct wrp_req_msg *req)
{
    WRPcode err = WRPE_OK;

    add_string(obj, WRP_DEST____.s, &req->dest, &err);
    add_string(obj, WRP_SOURCE__.s, &req->source, &err);
    add_bytes_(obj, WRP_PAYLOAD_.s, &req->payload, &err);
    add_string(obj, WRP_TRANS_ID.s, &req->trans_id, &err);
    add_string(obj, WRP_ACCEPT__.s, &req->accept, &err);
    add_string(obj, WRP_CT______.s, &req->content_type, &err);
    // headers
    // metadata
    add_string(obj, WRP_MSG_ID__.s, &req->msg_id, &err);
    // partners
    add_int___(obj, WRP_RDR_____.s, &req->rdr, &err);
    add_string(obj, WRP_SESS_ID_.s, &req->session_id, &err);
    add_int___(obj, WRP_STATUS__.s, &req->status, &err);

    return err;
}


static WRPcode handle_event(rbusObject_t obj, const struct wrp_event_msg *e)
{
    WRPcode err = WRPE_OK;

    add_string(obj, WRP_DEST____.s, &e->dest, &err);
    add_string(obj, WRP_SOURCE__.s, &e->source, &err);
    add_string(obj, WRP_CT______.s, &e->content_type, &err);
    // headers
    // metadata
    add_string(obj, WRP_MSG_ID__.s, &e->msg_id, &err);
    // partners
    add_bytes_(obj, WRP_PAYLOAD_.s, &e->payload, &err);
    add_string(obj, WRP_SESS_ID_.s, &e->session_id, &err);
    add_string(obj, WRP_TRANS_ID.s, &e->trans_id, &err);
    add_int___(obj, WRP_RDR_____.s, &e->rdr, &err);

    return err;
}


static WRPcode handle_crud(rbusObject_t obj, const struct wrp_crud_msg *crud)
{
    WRPcode err = WRPE_OK;

    add_string(obj, WRP_DEST____.s, &crud->dest, &err);
    add_string(obj, WRP_SOURCE__.s, &crud->source, &err);
    add_string(obj, WRP_TRANS_ID.s, &crud->trans_id, &err);

    add_string(obj, WRP_ACCEPT__.s, &crud->accept, &err);
    add_string(obj, WRP_CT______.s, &crud->content_type, &err);
    // headers
    // metadata
    add_string(obj, WRP_MSG_ID__.s, &crud->msg_id, &err);
    // partners
    add_string(obj, WRP_PATH____.s, &crud->path, &err);
    add_bytes_(obj, WRP_PAYLOAD_.s, &crud->payload, &err);
    add_int___(obj, WRP_RDR_____.s, &crud->rdr, &err);
    add_string(obj, WRP_SESS_ID_.s, &crud->session_id, &err);
    add_int___(obj, WRP_STATUS__.s, &crud->status, &err);

    return err;
}


static WRPcode handle_reg(rbusObject_t obj, const struct wrp_svc_reg_msg *reg)
{
    WRPcode err = WRPE_OK;

    add_string(obj, WRP_SN______.s, &reg->service_name, &err);
    add_string(obj, WRP_URL_____.s, &reg->service_name, &err);

    return err;
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

WRPcode wrp_to_rbus(const wrp_msg_t *src, rbusObject_t *dest)
{
    const char *msg_type;
    WRPcode err = WRPE_OK;

    if (!dest || !src) {
        return WRPE_INVALID_ARGS;
    }

    rbusObject_Init(dest, NULL);
    rbusObject_SetName(*dest, "wrp");

    switch (src->msg_type) {
        case WRP_MSG_TYPE__AUTH:
            msg_type = "auth";
            err      = handle_auth(*dest, &src->u.auth);
            break;
        case WRP_MSG_TYPE__REQ:
            msg_type = "reqresp";
            err      = handle_req(*dest, &src->u.req);
            break;
        case WRP_MSG_TYPE__EVENT:
            msg_type = "event";
            err      = handle_event(*dest, &src->u.event);
            break;
        case WRP_MSG_TYPE__CREATE:
            msg_type = "create";
            err      = handle_crud(*dest, &src->u.crud);
            break;
        case WRP_MSG_TYPE__RETRIEVE:
            msg_type = "retrieve";
            err      = handle_crud(*dest, &src->u.crud);
            break;
        case WRP_MSG_TYPE__UPDATE:
            msg_type = "update";
            err      = handle_crud(*dest, &src->u.crud);
            break;
        case WRP_MSG_TYPE__DELETE:
            msg_type = "delete";
            err      = handle_crud(*dest, &src->u.crud);
            break;
        case WRP_MSG_TYPE__SVC_REG:
            msg_type = "svc_reg";
            err      = handle_reg(*dest, &src->u.reg);
            break;
        case WRP_MSG_TYPE__SVC_ALIVE:
            msg_type = "svc_alive";
            break;
        default:
            return WRPE_INVALID_ARGS;
    }

    add_msg_type(*dest, msg_type, &err);

    if (WRPE_OK != err) {
        // TODO Cleanup
    }

    return err;
}


WRPcode wrp_from_rbus(const rbusObject_t src, wrp_msg_t **dest)
{
    // TODO for now
    (void) src;
    (void) dest;

    return WRPE_OK;
}
