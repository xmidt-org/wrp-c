/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv = WRPE_OK,
    .wrp_to_string_rv = WRPE_OK,

    .in.msg_type                = WRP_MSG_TYPE__EVENT,
    .in.u.event.source.s          = "source-address",
    .in.u.event.source.len        = 14,
    .in.u.event.dest.s            = "dest-address",
    .in.u.event.dest.len          = 12,
    .in.u.event.content_type.s    = "application/json",
    .in.u.event.content_type.len  = 16,
    .in.u.event.payload.data      = (const uint8_t*) "event 1234",
    .in.u.event.payload.len       = 10,

    .string =
    "wrp_event_msg {\n"
    "    .trans_id      = ''\n"
    "    .source        = 'source-address'\n"
    "    .dest          = 'dest-address'\n"
    "    .partner_ids   = ''\n"
    "    .content_type  = 'application/json'\n"
    "    .payload (len) = 10\n"
    "    .metadata      = {}\n"
    "}\n",

    .msgpack_len = 101,
    .msgpack =
        "\x85"  /* 5 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""payload"          /* : */ "\xc4""\x0a" /* len 10 */
                                             "event 1234"
};
// clang-format on

const char *test_name = __FILE__;
