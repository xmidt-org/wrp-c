/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test passing a zero length array, and zero length strings */

// clang-format off
const struct wrp_nvp meta[3] = {
    { .name.s = "n1", .name.len = 2, .value.s = "v1",  .value.len = 2 },
    { .name.s = "n2", .name.len = 2, .value.s = NULL,  .value.len = 0 },
    { .name.s = "n3", .name.len = 2, .value.s = NULL,  .value.len = 0 },
};

const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                 = 4,
    .in.u.event.source.s         = "source-address",
    .in.u.event.source.len       = 14,
    .in.u.event.dest.s           = "dest-address",
    .in.u.event.dest.len         = 12,
    .in.u.event.metadata.count   = 3,
    .in.u.event.metadata.list    = (struct wrp_nvp*) meta,

    .string = "wrp_event_msg {\n"
              "    .dest          = 'dest-address'\n"
              "    .source        = 'source-address'\n"
              "     - - optional - -\n"
              "    .content_type  = ''\n"
              "    .headers       = []\n"
              "    .metadata      = {\n"
              "        .n1: 'v1'\n"
              "        .n2: ''\n"
              "        .n3: ''\n"
              "    }\n"
              "    .msg_id        = ''\n"
              "    .partner_ids   = []\n"
              "    .payload (len) = 0\n"
              "    .session_id    = ''\n"
              "}\n",

    .asymetric_active      = true,
    .asymetric_msgpack_len = 75,
    .asymetric_msgpack =
        "\x84"  /* 4 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa8""metadata"         /* : */ "\x83" // 3
                                                 "\xa2""n1" "\xa2""v1"
                                                 "\xa2""n2" "\xa0"
                                                 "\xa2""n3" "\xa0",
 
    .msgpack_len = 88,
    .msgpack =
        "\x85"  /* 5 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa6""source"           /* : */ "\xae""source-address"
            "\xa8""metadata"         /* : */ "\x83" // 3
                                                 "\xa2""n1" "\xa2""v1"
                                                 "\xa2""n2" "\xa0"
                                                 "\xa2""n3" "\xc0"
            "\xab""partner_ids"      /* : */ "\x90" // 0
};
// clang-format on

const char *test_name = __FILE__;
