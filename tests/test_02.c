/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test a minimual request message. */

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                = WRP_MSG_TYPE__REQ,
    .in.u.req.dest.s            = "dest-address",
    .in.u.req.dest.len          = 12,
    .in.u.req.payload.data      = (const uint8_t*) "123",
    .in.u.req.payload.len       = 3,
    .in.u.req.source.s          = "source-address",
    .in.u.req.source.len        = 14,
    .in.u.req.trans_id.s        = "c07ee5e1-70be-444c-a156-097c767ad8aa",
    .in.u.req.trans_id.len      = 36,

    .string = "wrp_req_msg {\n"
              "    .dest          = 'dest-address'\n"
              "    .payload (len) = 3\n"
              "    .source        = 'source-address'\n"
              "    .trans_id      = 'c07ee5e1-70be-444c-a156-097c767ad8aa'\n"
              "     - - optional - -\n"
              "    .accept        = ''\n"
              "    .content_type  = ''\n"
              "    .headers       = []\n"
              "    .metadata      = {}\n"
              "    .msg_id        = ''\n"
              "    .partner_ids   = []\n"
              "    .rdr           = ''\n"
              "    .session_id    = ''\n"
              "    .status        = ''\n"
              "}\n",

    .msgpack_len = 119,
    .msgpack =
        "\x85"  /* 5 name value pairs */
            "\xa8""msg_type"         /* : */ "\x03" // 3
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa7""payload"          /* : */ "\xc4""\x03" /* len 3 */
                                             "123"
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
};
// clang-format on

const char *test_name = __FILE__;
