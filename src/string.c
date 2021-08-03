/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/printf.h>

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
static void string_list_to_string(const struct wrp_string_list *list, char **dst, size_t *len)
{
    const char *comma = "";

    *dst = NULL;
    *len = 0;

    for (size_t i = 0; i < list->count; i++) {
        char *p = NULL;

        p = mlaprintf(len, "%.*s%s%.*s", (int)*len, *dst, comma, (int)list->list[i].len, list->list[i].s);
        free(*dst);
        *dst = p;
        if (!*dst) {
            return;
        }
        comma = ", ";
    }
}


static void nvp_list_to_string(const struct wrp_nvp_list *list, char **dst, size_t *len)
{
    char *p = NULL;
    const char *nl = "\n";

    *dst = NULL;
    *len = 0;

    for (size_t i = 0; i < list->count; i++) {
        p = mlaprintf(len, "%s%.*s        .%.*s: '%.*s'\n", nl, (int)*len, *dst,
                      (int)list->list[i].name.len, list->list[i].name.s,
                      (int)list->list[i].value.len, list->list[i].value.s);
        nl = "";
        free(*dst);
        *dst = p;
        if (!*dst) {
            return;
        }
    }

    if (0 < *len) {
        p = mlaprintf(len, "%s    ", *dst);
        free(*dst);
        *dst = p;
    }
}


static WRPcode auth_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    size_t _len;
    WRPcode rv = WRPE_OUT_OF_MEMORY;

    *dst = mlaprintf(&_len, "wrp_auth_msg {\n"
                            "    .status = %d\n"
                            "}\n",
                     msg->u.auth.status);

    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    return rv;
}


static WRPcode req_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv = WRPE_OUT_OF_MEMORY;
    const struct wrp_req_msg *req = &msg->u.req;
    char *partners = NULL;
    char *metadata = NULL;
    size_t partner_len = 0;
    size_t metadata_len = 0;
    size_t _len = 0;

    string_list_to_string(&req->partner_ids, &partners, &partner_len);
    nvp_list_to_string(&req->metadata, &metadata, &metadata_len);

    *dst = mlaprintf(&_len, "wrp_req_msg {\n"
                            "    .trans_id      = '%.*s'\n"
                            "    .source        = '%.*s'\n"
                            "    .dest          = '%.*s'\n"
                            "    .partner_ids   = '%.*s'\n"
                            "    .content_type  = '%.*s'\n"
                            "    .accept        = '%.*s'\n"
                            "    .payload (len) = %zd\n"
                            "    .metadata      = {%.*s}\n"
                            "}\n",
                     (int)req->trans_id.len, req->trans_id.s,
                     (int)req->source.len, req->source.s,
                     (int)req->dest.len, req->dest.s,
                     (int)partner_len, partners,
                     (int)req->content_type.len, req->content_type.s,
                     (int)req->accept.len, req->accept.s,
                     req->payload.len,
                     (int)metadata_len, metadata);

    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    if (partners) free(partners);
    if (metadata) free(metadata);

    return rv;
}


static WRPcode event_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv = WRPE_OUT_OF_MEMORY;
    const struct wrp_event_msg *event = &msg->u.event;
    char *partners = NULL;
    char *metadata = NULL;
    size_t partner_len = 0;
    size_t metadata_len = 0;
    size_t _len = 0;

    string_list_to_string(&event->partner_ids, &partners, &partner_len);
    nvp_list_to_string(&event->metadata, &metadata, &metadata_len);

    *dst = mlaprintf(&_len, "wrp_event_msg {\n"
                            "    .trans_id      = '%.*s'\n"
                            "    .source        = '%.*s'\n"
                            "    .dest          = '%.*s'\n"
                            "    .partner_ids   = '%.*s'\n"
                            "    .content_type  = '%.*s'\n"
                            "    .payload (len) = %zd\n"
                            "    .metadata      = {%.*s}\n"
                            "}\n",
                     (int)event->trans_id.len, event->trans_id.s,
                     (int)event->source.len, event->source.s,
                     (int)event->dest.len, event->dest.s,
                     (int)partner_len, partners,
                     (int)event->content_type.len, event->content_type.s,
                     event->payload.len,
                     (int)metadata_len, metadata);

    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    if (partners) free(partners);
    if (metadata) free(metadata);

    return rv;
}


static WRPcode crud_to_string(const wrp_msg_t *msg, char **dst, size_t *len,
                              const char *type)
{
    WRPcode rv = WRPE_OUT_OF_MEMORY;
    const struct wrp_crud_msg *crud = &msg->u.crud;
    char *partners = NULL;
    char *metadata = NULL;
    size_t partner_len = 0;
    size_t metadata_len = 0;
    size_t _len = 0;

    string_list_to_string(&crud->partner_ids, &partners, &partner_len);
    nvp_list_to_string(&crud->metadata, &metadata, &metadata_len);

    *dst = mlaprintf(&_len, "wrp_crud_msg (%s) {\n"
                            "    .trans_id      = '%.*s'\n"
                            "    .source        = '%.*s'\n"
                            "    .dest          = '%.*s'\n"
                            "    .partner_ids   = '%.*s'\n"
                            "    .content_type  = '%.*s'\n"
                            "    .accept        = '%.*s'\n"
                            "    .status        = %d\n"
                            "    .rdr           = %d\n"
                            "    .path          = '%.*s'\n"
                            "    .payload (len) = %zd\n"
                            "    .metadata      = {%.*s}\n"
                            "}\n",
                     type,
                     (int)crud->trans_id.len, crud->trans_id.s,
                     (int)crud->source.len, crud->source.s,
                     (int)crud->dest.len, crud->dest.s,
                     (int)partner_len, partners,
                     (int)crud->content_type.len, crud->content_type.s,
                     (int)crud->accept.len, crud->accept.s,
                     crud->status,
                     crud->rdr,
                     (int)crud->path.len, crud->path.s,
                     crud->payload.len,
                     (int)metadata_len, metadata);

    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    if (partners) free(partners);
    if (metadata) free(metadata);

    return rv;
}


static WRPcode reg_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv = WRPE_OUT_OF_MEMORY;
    const struct wrp_svc_reg_msg *reg = &msg->u.reg;
    size_t _len = 0;

    *dst = mlaprintf(&_len, "wrp_svc_reg_msg {\n"
                            "    .service_name = '%.*s'\n"
                            "    .url          = '%.*s'\n"
                            "}\n",
                     (int)reg->service_name.len, reg->service_name.s,
                     (int)reg->url.len, reg->url.s);

    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    return rv;
}


static WRPcode keep_alive_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    size_t _len;
    WRPcode rv = WRPE_OUT_OF_MEMORY;

    (void)msg;

    *dst = mlaprintf(&_len, "wrp_keep_alive_msg {}\n");
    if (NULL != *dst) {
        *len = _len;
        rv = WRPE_OK;
    }

    return rv;
}


WRPcode wrp_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    size_t _len = 0;
    WRPcode rv = WRPE_OK;

    if (!msg || !dst) {
        return WRPE_INVALID_ARGS;
    }

    switch (msg->msg_type) {
    case WRP_MSG_TYPE__AUTH:
        rv = auth_to_string(msg, dst, &_len);
        break;
    case WRP_MSG_TYPE__REQ:
        rv = req_to_string(msg, dst, &_len);
        break;
    case WRP_MSG_TYPE__EVENT:
        rv = event_to_string(msg, dst, &_len);
        break;
    case WRP_MSG_TYPE__CREATE:
        rv = crud_to_string(msg, dst, &_len, "CREATE");
        break;
    case WRP_MSG_TYPE__RETRIEVE:
        rv = crud_to_string(msg, dst, &_len, "RETRIEVE");
        break;
    case WRP_MSG_TYPE__UPDATE:
        rv = crud_to_string(msg, dst, &_len, "UPDATE");
        break;
    case WRP_MSG_TYPE__DELETE:
        rv = crud_to_string(msg, dst, &_len, "DELETE");
        break;
    case WRP_MSG_TYPE__SVC_REG:
        rv = reg_to_string(msg, dst, &_len);
        break;
    case WRP_MSG_TYPE__SVC_ALIVE:
        rv = keep_alive_to_string(msg, dst, &_len);
        break;
    default:
        rv = WRPE_NOT_A_WRP_MSG;
        break;
    }

    if (len) {
        *len = _len;
    }

    return rv;
}


WRPcode wrp_loc_to_string(const wrp_locator_t *loc, char **dst, size_t *len)
{
    WRPcode rv = WRPE_OUT_OF_MEMORY;
    size_t _len = 0;

    if (!loc || !dst) {
        return WRPE_INVALID_ARGS;
    }

    *dst = mlaprintf(&_len, "wrp_locator_t {\n"
                            "    .scheme    = '%.*s'\n"
                            "    .authority = '%.*s'\n"
                            "    .service   = '%.*s'\n"
                            "    .app       = '%.*s'\n"
                            "}\n",
                     (int)loc->scheme.len, loc->scheme.s,
                     (int)loc->authority.len, loc->authority.s,
                     (int)loc->service.len, loc->service.s,
                     (int)loc->app.len, loc->app.s);

    if (NULL != *dst) {
        if (len) {
            *len = _len;
        }
        rv = WRPE_OK;
    }

    return rv;
}
