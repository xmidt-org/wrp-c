/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct wrp_string list[3] = {
    { .s = "Partner 1", .len = 9 },
    { .s = "Partner 2", .len = 9 },
    { .s = "Example",   .len = 7 },
};

const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv = WRPE_OK,
    .wrp_to_string_rv = WRPE_OK,

    .in.msg_type                = WRP_MSG_TYPE__UPDATE,
    .in.u.crud.trans_id.s        = "c07ee5e1-70be-444c-a156-097c767ad8aa",
    .in.u.crud.trans_id.len      = 36,
    .in.u.crud.source.s          = "source-address",
    .in.u.crud.source.len        = 14,
    .in.u.crud.dest.s            = "dest-address",
    .in.u.crud.dest.len          = 12,
    .in.u.crud.partner_ids.count = 3,
    .in.u.crud.partner_ids.list  = (struct wrp_string*) list,
    .in.u.crud.content_type.s    = "application/json",
    .in.u.crud.content_type.len  = 16,
    .in.u.crud.accept.s          = "json",
    .in.u.crud.accept.len        = 4,
    .in.u.crud.path.s            = "the_path",
    .in.u.crud.path.len          = 8,
    .in.u.crud.payload.data      = (const uint8_t*) "crud 1234",
    .in.u.crud.payload.len       = 9,
    .in.u.crud.rdr               = 12,
    .in.u.crud.status            = 200,

    .string = "wrp_crud_msg (UPDATE) {\n"
              "    .trans_id      = 'c07ee5e1-70be-444c-a156-097c767ad8aa'\n"
              "    .source        = 'source-address'\n"
              "    .dest          = 'dest-address'\n"
              "    .partner_ids   = 'Partner 1, Partner 2, Example'\n"
              "    .content_type  = 'application/json'\n"
              "    .accept        = 'json'\n"
              "    .status        = 200\n"
              "    .rdr           = 12\n"
              "    .path          = 'the_path'\n"
              "    .payload (len) = 9\n"
              "    .metadata      = {}\n"
              "}\n",

    .msgpack_len = 236,
    .msgpack =
        "\x8b"  /* 10 name value pairs */
            "\xa8""msg_type"         /* : */ "\x07" // 7
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xab""partner_ids"      /* : */ "\x93" // 3
                                                 "\xa9""Partner 1"
                                                 "\xa9""Partner 2"
                                                 "\xa7""Example"
            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
            "\xa6""status"           /* : */ "\xcc""\xc8" /* 200 */
            "\xa3""rdr"              /* : */ "\x0c"       /* 12 */
            "\xa4""path"             /* : */ "\xa8""the_path"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa6""accept"           /* : */ "\xa4""json"
            "\xa7""payload"          /* : */ "\xc4""\x09" /* len 9 */
                                             "crud 1234"
};
// clang-format on

const char *test_name = __FILE__;
