/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_NOT_A_WRP_MSG,
    .wrp_to_msgpack_rv = WRPE_NOT_A_WRP_MSG,
    .wrp_to_string_rv = WRPE_NOT_A_WRP_MSG,

    .in.msg_type = 0,

    .msgpack_len = 11,
    .msgpack = "\x81"
                   "\xa8msg_type"  /* : */ "\xc0"
};
// clang-format on

const char *test_name = __FILE__;
