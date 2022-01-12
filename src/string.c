/* SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC */
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
struct wrp_str {
    size_t len;
    char *s;
};

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
    char *p           = NULL;
    const char *nl    = "\n";
    const char *comma = ",";

    *dst = NULL;
    *len = 0;

    for (size_t i = 0; i < list->count; i++) {
        if (i == (list->count - 1)) {
            comma = "";
        }
        p  = mlaprintf(len, "%s%.*s        '%.*s'%s\n", nl, (int) *len, *dst,
                       (int) list->list[i].len, list->list[i].s, comma);
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


static void nvp_list_to_string(const struct wrp_nvp_list *list, char **dst, size_t *len)
{
    char *p        = NULL;
    const char *nl = "\n";

    *dst = NULL;
    *len = 0;

    for (size_t i = 0; i < list->count; i++) {
        p  = mlaprintf(len, "%s%.*s        .%.*s: '%.*s'\n", nl, (int) *len, *dst,
                       (int) list->list[i].name.len, list->list[i].name.s,
                       (int) list->list[i].value.len, list->list[i].value.s);
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

    *dst = mlaprintf(&_len,
                     "wrp_auth_msg {\n"
                     "    .status = '%.*d'\n"
                     "}\n",
                     (msg->u.auth.status.num) ? 1 : 0,
                     (msg->u.auth.status.num) ? *msg->u.auth.status.num : 0);

    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    return rv;
}


static WRPcode req_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv                    = WRPE_OUT_OF_MEMORY;
    const struct wrp_req_msg *req = &msg->u.req;
    struct wrp_str headers        = { 0, NULL };
    struct wrp_str partners       = { 0, NULL };
    struct wrp_str metadata       = { 0, NULL };
    size_t _len                   = 0;

    string_list_to_string(&req->headers, &headers.s, &headers.len);
    string_list_to_string(&req->partner_ids, &partners.s, &partners.len);
    nvp_list_to_string(&req->metadata, &metadata.s, &metadata.len);

    *dst = mlaprintf(&_len,
                     "wrp_req_msg {\n"
                     "    .dest          = '%.*s'\n"
                     "    .payload (len) = %zd\n"
                     "    .source        = '%.*s'\n"
                     "    .trans_id      = '%.*s'\n"
                     "     - - optional - -\n"
                     "    .accept        = '%.*s'\n"
                     "    .content_type  = '%.*s'\n"
                     "    .headers       = [%.*s]\n"
                     "    .metadata      = {%.*s}\n"
                     "    .msg_id        = '%.*s'\n"
                     "    .partner_ids   = [%.*s]\n"
                     "    .rdr           = '%.*d'\n"
                     "    .session_id    = '%.*s'\n"
                     "    .status        = '%.*d'\n"
                     "}\n",
                     (int) req->dest.len, req->dest.s,
                     req->payload.len,
                     (int) req->source.len, req->source.s,
                     (int) req->trans_id.len, req->trans_id.s,

                     (int) req->accept.len, req->accept.s,
                     (int) req->content_type.len, req->content_type.s,
                     (int) headers.len, headers.s,
                     (int) metadata.len, metadata.s,
                     (int) req->msg_id.len, req->msg_id.s,
                     (int) partners.len, partners.s,
                     (req->rdr.num) ? 1 : 0, (req->rdr.num) ? *req->rdr.num : 0,
                     (int) req->session_id.len, req->session_id.s,
                     (req->status.num) ? 1 : 0, (req->status.num) ? *req->status.num : 0);

    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    if (headers.s) free(headers.s);
    if (partners.s) free(partners.s);
    if (metadata.s) free(metadata.s);

    return rv;
}


static WRPcode event_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv                        = WRPE_OUT_OF_MEMORY;
    const struct wrp_event_msg *event = &msg->u.event;
    struct wrp_str headers            = { 0, NULL };
    struct wrp_str partners           = { 0, NULL };
    struct wrp_str metadata           = { 0, NULL };
    size_t _len                       = 0;

    string_list_to_string(&event->headers, &headers.s, &headers.len);
    string_list_to_string(&event->partner_ids, &partners.s, &partners.len);
    nvp_list_to_string(&event->metadata, &metadata.s, &metadata.len);

    *dst = mlaprintf(&_len,
                     "wrp_event_msg {\n"
                     "    .dest          = '%.*s'\n"
                     "    .source        = '%.*s'\n"
                     "     - - optional - -\n"
                     "    .content_type  = '%.*s'\n"
                     "    .headers       = [%.*s]\n"
                     "    .metadata      = {%.*s}\n"
                     "    .msg_id        = '%.*s'\n"
                     "    .partner_ids   = [%.*s]\n"
                     "    .payload (len) = %zd\n"
                     "    .session_id    = '%.*s'\n"
                     "}\n",
                     (int) event->dest.len, event->dest.s,
                     (int) event->source.len, event->source.s,

                     (int) event->content_type.len, event->content_type.s,
                     (int) headers.len, headers.s,
                     (int) metadata.len, metadata.s,
                     (int) event->msg_id.len, event->msg_id.s,
                     (int) partners.len, partners.s,
                     event->payload.len,
                     (int) event->session_id.len, event->session_id.s);

    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    if (headers.s) free(headers.s);
    if (partners.s) free(partners.s);
    if (metadata.s) free(metadata.s);

    return rv;
}


static WRPcode crud_to_string(const wrp_msg_t *msg, char **dst, size_t *len,
                              const char *type)
{
    WRPcode rv                      = WRPE_OUT_OF_MEMORY;
    const struct wrp_crud_msg *crud = &msg->u.crud;
    struct wrp_str headers          = { 0, NULL };
    struct wrp_str partners         = { 0, NULL };
    struct wrp_str metadata         = { 0, NULL };
    size_t _len                     = 0;

    string_list_to_string(&crud->headers, &headers.s, &headers.len);
    string_list_to_string(&crud->partner_ids, &partners.s, &partners.len);
    nvp_list_to_string(&crud->metadata, &metadata.s, &metadata.len);

    *dst = mlaprintf(&_len,
                     "wrp_crud_msg (%s) {\n"
                     "    .dest          = '%.*s'\n"
                     "    .source        = '%.*s'\n"
                     "    .trans_id      = '%.*s'\n"
                     "     - - optional - -\n"
                     "    .accept        = '%.*s'\n"
                     "    .content_type  = '%.*s'\n"
                     "    .headers       = [%.*s]\n"
                     "    .metadata      = {%.*s}\n"
                     "    .msg_id        = '%.*s'\n"
                     "    .partner_ids   = [%.*s]\n"
                     "    .path          = '%.*s'\n"
                     "    .payload (len) = %zd\n"
                     "    .rdr           = '%.*d'\n"
                     "    .session_id    = '%.*s'\n"
                     "    .status        = '%.*d'\n"
                     "}\n",
                     type,
                     (int) crud->dest.len, crud->dest.s,
                     (int) crud->source.len, crud->source.s,
                     (int) crud->trans_id.len, crud->trans_id.s,
                     (int) crud->accept.len, crud->accept.s,
                     (int) crud->content_type.len, crud->content_type.s,
                     (int) headers.len, headers.s,
                     (int) metadata.len, metadata.s,
                     (int) crud->msg_id.len, crud->msg_id.s,
                     (int) partners.len, partners.s,
                     (int) crud->path.len, crud->path.s,
                     crud->payload.len,
                     (crud->rdr.num) ? 1 : 0, (crud->rdr.num) ? *crud->rdr.num : 0,
                     (int) crud->session_id.len, crud->session_id.s,
                     (crud->status.num) ? 1 : 0, (crud->status.num) ? *crud->status.num : 0);

    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    if (headers.s) free(headers.s);
    if (partners.s) free(partners.s);
    if (metadata.s) free(metadata.s);

    return rv;
}


static WRPcode reg_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    WRPcode rv                        = WRPE_OUT_OF_MEMORY;
    const struct wrp_svc_reg_msg *reg = &msg->u.reg;
    size_t _len                       = 0;

    *dst = mlaprintf(&_len,
                     "wrp_svc_reg_msg {\n"
                     "    .service_name = '%.*s'\n"
                     "    .url          = '%.*s'\n"
                     "}\n",
                     (int) reg->service_name.len, reg->service_name.s,
                     (int) reg->url.len, reg->url.s);

    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    return rv;
}


static WRPcode keep_alive_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    size_t _len;
    WRPcode rv = WRPE_OUT_OF_MEMORY;

    (void) msg;

    *dst = mlaprintf(&_len, "wrp_keep_alive_msg {}\n");
    if (NULL != *dst) {
        *len = _len;
        rv   = WRPE_OK;
    }

    return rv;
}


WRPcode wrp_to_string(const wrp_msg_t *msg, char **dst, size_t *len)
{
    size_t _len = 0;
    WRPcode rv  = WRPE_OK;

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
    WRPcode rv  = WRPE_OUT_OF_MEMORY;
    size_t _len = 0;

    if (!loc || !dst) {
        return WRPE_INVALID_ARGS;
    }

    *dst = mlaprintf(&_len,
                     "wrp_locator_t {\n"
                     "    .scheme    = '%.*s'\n"
                     "    .authority = '%.*s'\n"
                     "    .service   = '%.*s'\n"
                     "    .app       = '%.*s'\n"
                     "}\n",
                     (int) loc->scheme.len, loc->scheme.s,
                     (int) loc->authority.len, loc->authority.s,
                     (int) loc->service.len, loc->service.s,
                     (int) loc->app.len, loc->app.s);

    if (NULL != *dst) {
        if (len) {
            *len = _len;
        }
        rv = WRPE_OK;
    }

    return rv;
}
