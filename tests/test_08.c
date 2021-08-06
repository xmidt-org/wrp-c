/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

/* Test a keep alive message. */

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv   = WRPE_OK,
    .wrp_to_string_rv    = WRPE_OK,

    .in.msg_type = WRP_MSG_TYPE__SVC_ALIVE,

 
    .string = "wrp_keep_alive_msg {}\n",

    .msgpack_len = 11,
    .msgpack =
        "\x81"  /* 1 name value pairs */
            "\xa8""msg_type"    /* : */ "\x0a" // 10
};
// clang-format on

const char *test_name = __FILE__;
