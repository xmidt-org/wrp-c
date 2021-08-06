/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test a fully defined auth message. */

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type            = WRP_MSG_TYPE__AUTH,
    .in.u.auth.status.valid = true,
    .in.u.auth.status.n     = 123,

    .string = "wrp_auth_msg {\n"
              "    .status = '123'\n"
              "}\n",

    .msgpack_len = 19,
    .msgpack =
        "\x82"  /* 2 name value pairs */
            "\xa8""msg_type" /* : */ "\x02"
            "\xa6""status"   /* : */ "\x7b" // 123
};
// clang-format on

const char *test_name = __FILE__;
