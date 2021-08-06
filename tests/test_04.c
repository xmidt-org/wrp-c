/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test a fully defined event message. */

// clang-format off
const struct wrp_string parts[3] = {
    { .s = "Partner 1", .len = 9 },
    { .s = "Partner 2", .len = 9 },
    { .s = "Example",   .len = 7 },
};

const struct wrp_nvp meta[1] = {
    { .name.s = "n1", .name.len = 2, .value.s = "v1",  .value.len = 2 },
};

const struct wrp_string heads[1] = {
    { .s = "b3: 80f198ee56343ba864fe8b2a57d3eff7-e457b5a2e4d86bd1-1-05e3ac9a4f6e3b90", .len = 72 },
};

const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type                  = WRP_MSG_TYPE__EVENT,
    .in.u.event.dest.s            = "dest-address",
    .in.u.event.dest.len          = 12,
    .in.u.event.source.s          = "source-address",
    .in.u.event.source.len        = 14,

    .in.u.event.content_type.s    = "application/json",
    .in.u.event.content_type.len  = 16,
    .in.u.event.headers.count     = sizeof(heads)/sizeof(struct wrp_string),
    .in.u.event.headers.list      = (struct wrp_string*) heads,
    .in.u.event.metadata.count    = sizeof(meta)/sizeof(struct wrp_nvp),
    .in.u.event.metadata.list     = (struct wrp_nvp*) meta,
    .in.u.event.msg_id.s          = "ec704e3d-23ec-4dd6-b8d4-e2cb8bedb521",
    .in.u.event.msg_id.len        = 36,
    .in.u.event.partner_ids.count = sizeof(parts)/sizeof(struct wrp_string),
    .in.u.event.partner_ids.list  = (struct wrp_string*) parts,
    .in.u.event.payload.data      = (const uint8_t*) "123",
    .in.u.event.payload.len       = 3,
    .in.u.event.session_id.s      = "bebc43de-501b-4435-a8b1-8a4d174a1a91",
    .in.u.event.session_id.len    = 36,

    .string = "wrp_event_msg {\n"
              "    .dest          = 'dest-address'\n"
              "    .source        = 'source-address'\n"
              "     - - optional - -\n"
              "    .content_type  = 'application/json'\n"
              "    .headers       = [\n"
              "        'b3: 80f198ee56343ba864fe8b2a57d3eff7-e457b5a2e4d86bd1-1-05e3ac9a4f6e3b90'\n"
              "    ]\n"
              "    .metadata      = {\n"
              "        .n1: 'v1'\n"
              "    }\n"
              "    .msg_id        = 'ec704e3d-23ec-4dd6-b8d4-e2cb8bedb521'\n"
              "    .partner_ids   = [\n"
              "        'Partner 1',\n"
              "        'Partner 2',\n"
              "        'Example'\n"
              "    ]\n"
              "    .payload (len) = 3\n"
              "    .session_id    = 'bebc43de-501b-4435-a8b1-8a4d174a1a91'\n"
              "}\n",

    .msgpack_len = 328,
    .msgpack =
        "\x8a"  /* 10 name value pairs */
            "\xa8""msg_type"         /* : */ "\x04" // 4
            "\xa4""dest"             /* : */ "\xac""dest-address"
            "\xa6""source"           /* : */ "\xae""source-address"

            "\xac""content_type"     /* : */ "\xb0""application/json"
            "\xa7""headers"          /* : */ "\x91" // 1
                                                 "\xd9\x48""b3: 80f198ee56343ba864fe8b2a57d3eff7-e457b5a2e4d86bd1-1-05e3ac9a4f6e3b90"
            "\xa8""metadata"         /* : */ "\x81" // 1
                                                 "\xa2""n1" "\xa2""v1"
            "\xa6""msg_id"           /* : */ "\xd9\x24""ec704e3d-23ec-4dd6-b8d4-e2cb8bedb521"
            "\xab""partner_ids"      /* : */ "\x93" // 3
                                                 "\xa9""Partner 1"
                                                 "\xa9""Partner 2"
                                                 "\xa7""Example"
            "\xa7""payload"          /* : */ "\xc4""\x03" /* len 3 */
                                             "123"
            "\xaa""session_id"       /* : */ "\xd9\x24""bebc43de-501b-4435-a8b1-8a4d174a1a91"
};
// clang-format on

const char *test_name = __FILE__;
