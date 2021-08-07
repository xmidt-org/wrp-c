/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test a minimumal event message */

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                  = WRP_MSG_TYPE__EVENT,
    .in.u.event.source.s          = "source-address",
    .in.u.event.source.len        = 14,
    .in.u.event.dest.s            = "dest-address",
    .in.u.event.dest.len          = 12,

    .string = "wrp_event_msg {\n"
              "    .dest          = 'dest-address'\n"
              "    .source        = 'source-address'\n"
              "     - - optional - -\n"
              "    .content_type  = ''\n"
              "    .headers       = []\n"
              "    .metadata      = {}\n"
              "    .msg_id        = ''\n"
              "    .partner_ids   = []\n"
              "    .payload (len) = 0\n"
              "    .session_id    = ''\n"
              "}\n",

    .msgpack_len = 51,
    .msgpack =
        "\x83"  /* 3 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa6""source"           /* : */ "\xae""source-address"
};
// clang-format on

const char *test_name = __FILE__;
