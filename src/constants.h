/* SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <stddef.h>

struct wrp_token {
    const char *s;
    size_t len;
};

extern const struct wrp_token WRP_ACCEPT__;
extern const struct wrp_token WRP_CT______;
extern const struct wrp_token WRP_DEST____;
extern const struct wrp_token WRP_HEADERS_;
extern const struct wrp_token WRP_METADATA;
extern const struct wrp_token WRP_MSG_ID__;
extern const struct wrp_token WRP_MSG_TYPE;
extern const struct wrp_token WRP_PARTNERS;
extern const struct wrp_token WRP_PATH____;
extern const struct wrp_token WRP_PAYLOAD_;
extern const struct wrp_token WRP_RDR_____;
extern const struct wrp_token WRP_SESS_ID_;
extern const struct wrp_token WRP_SN______;
extern const struct wrp_token WRP_SOURCE__;
extern const struct wrp_token WRP_STATUS__;
extern const struct wrp_token WRP_TRANS_ID;
extern const struct wrp_token WRP_URL_____;

#endif
