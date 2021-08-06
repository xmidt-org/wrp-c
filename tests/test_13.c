/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test out the retrieve crud message. */

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                 = WRP_MSG_TYPE__RETRIEVE,
    .in.u.crud.dest.s            = "dest-address",
    .in.u.crud.dest.len          = 12,
    .in.u.crud.source.s          = "source-address",
    .in.u.crud.source.len        = 14,
    .in.u.crud.trans_id.s        = "c07ee5e1-70be-444c-a156-097c767ad8aa",
    .in.u.crud.trans_id.len      = 36,

    .string = "wrp_crud_msg (RETRIEVE) {\n"
              "    .dest          = 'dest-address'\n"
              "    .source        = 'source-address'\n"
              "    .trans_id      = 'c07ee5e1-70be-444c-a156-097c767ad8aa'\n"
              "     - - optional - -\n"
              "    .accept        = ''\n"
              "    .content_type  = ''\n"
              "    .headers       = []\n"
              "    .metadata      = {}\n"
              "    .msg_id        = ''\n"
              "    .partner_ids   = []\n"
              "    .path          = ''\n"
              "    .payload (len) = 0\n"
              "    .rdr           = ''\n"
              "    .session_id    = ''\n"
              "    .status        = ''\n"
              "}\n",

    .msgpack_len = 106,
    .msgpack =
        "\x84"  /* 4 name value pairs */
            "\xa8""msg_type"         /* : */ "\x06" // 6
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
};
// clang-format on

const char *test_name = __FILE__;
