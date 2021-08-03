/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct wrp_nvp meta[3] = {
    { .name.s = "n1", .name.len = 2, .value.s = "v1",  .value.len = 2 },
    { .name.s = "n2", .name.len = 2, .value.s = NULL,  .value.len = 0 },
    { .name.s = "n3", .name.len = 2, .value.s = NULL,  .value.len = 0 },
};

const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv = WRPE_OK,
    .wrp_to_string_rv = WRPE_OK,

    .in.msg_type                  = 4,
    .in.u.event.trans_id.s        = "c07ee5e1-70be-444c-a156-097c767ad8aa",
    .in.u.event.trans_id.len      = 36,
    .in.u.event.source.s          = "source-address",
    .in.u.event.source.len        = 14,
    .in.u.event.dest.s            = "dest-address",
    .in.u.event.dest.len          = 12,
    .in.u.event.content_type.s    = "application/json",
    .in.u.event.content_type.len  = 16,
    .in.u.event.payload.data      = (const uint8_t*) "event 1234",
    .in.u.event.payload.len       = 10,
    .in.u.event.metadata.count    = 3,
    .in.u.event.metadata.list     = (struct wrp_nvp*) meta,

    .string = "wrp_event_msg {\n"
              "    .trans_id      = 'c07ee5e1-70be-444c-a156-097c767ad8aa'\n"
              "    .source        = 'source-address'\n"
              "    .dest          = 'dest-address'\n"
              "    .partner_ids   = ''\n"
              "    .content_type  = 'application/json'\n"
              "    .payload (len) = 10\n"
              "    .metadata      = {\n"
              "        .n1: 'v1'\n"
              "        .n2: ''\n"
              "        .n3: ''\n"
              "    }\n"
              "}\n",

    .asymetric_active = true,
    .asymetric_msgpack_len = 180,
    .asymetric_msgpack =
        "\x87"  /* 7 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa8""metadata"         /* : */ "\x83" // 3
                                                 "\xa2""n1" "\xa2""v1"
                                                 "\xa2""n2" "\xa0"
                                                 "\xa2""n3" "\xa0"

            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""payload"          /* : */ "\xc4""\x0a" /* len 10 */
                                             "event 1234",
 
    .msgpack_len = 193,
    .msgpack =
        "\x88"  /* 8 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xab""partner_ids"      /* : */ "\x90" // 0
            "\xa8""metadata"         /* : */ "\x83" // 3
                                                 "\xa2""n1" "\xa2""v1"
                                                 "\xa2""n2" "\xa0"
                                                 "\xa2""n3" "\xc0"

            "\xb0""transaction_uuid" /* : */ "\xd9\x24""c07ee5e1-70be-444c-a156-097c767ad8aa"
            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""payload"          /* : */ "\xc4""\x0a" /* len 10 */
                                             "event 1234"
};
// clang-format on

const char *test_name = __FILE__;
