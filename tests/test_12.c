/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                 = WRP_MSG_TYPE__EVENT,
    .in.u.event.trans_id.s       = "c07ee5e1-70be-444c-a156-097c767ad8aa",
    .in.u.event.trans_id.len     = 36,
    .in.u.event.source.s         = "source-address",
    .in.u.event.source.len       = 14,
    .in.u.event.dest.s           = "dest-address",
    .in.u.event.dest.len         = 12,
    .in.u.event.content_type.s   = "application/json",
    .in.u.event.content_type.len = 16,
    .in.u.event.payload.data     = (const uint8_t*) "event 1234",
    .in.u.event.payload.len      = 10,

    .string = "wrp_event_msg {\n"
              "    .trans_id      = 'c07ee5e1-70be-444c-a156-097c767ad8aa'\n"
              "    .source        = 'source-address'\n"
              "    .dest          = 'dest-address'\n"
              "    .partner_ids   = ''\n"
              "    .content_type  = 'application/json'\n"
              "    .payload (len) = 10\n"
              "    .metadata      = {}\n"
              "}\n",

    .asymetric_active      = true,
    .asymetric_msgpack_len = 156,
    .asymetric_msgpack =
        "\x86"  /* 6 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"

            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""payload"          /* : */ "\xc4""\x0a" /* len 10 */
                                             "event 1234",
    .msgpack_len = 166,
    .msgpack =
        "\x87"  /* 7 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa8""metadata"         /* : */ "\x80" // 0

            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""payload"          /* : */ "\xc4""\x0a" /* len 10 */
                                             "event 1234"
};
// clang-format on

const char *test_name = __FILE__;
