/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "test_common.h"

// clang-format off
const struct test_vector test = {
    .wrp_from_msgpack_rv = WRPE_OK,
    .wrp_to_msgpack_rv = WRPE_OK,
    .wrp_to_string_rv = WRPE_OK,

    .in.msg_type                = WRP_MSG_TYPE__SVC_REG,
    .in.u.reg.service_name.s    = "service name",
    .in.u.reg.service_name.len  = 12,
    .in.u.reg.url.s             = "url://url",
    .in.u.reg.url.len           = 9,

 
    .string = "wrp_svc_reg_msg {\n"
              "    .service_name = 'service name'\n"
              "    .url          = 'url://url'\n"
              "}\n",

    .msgpack_len = 51,
    .msgpack =
        "\x83"  /* 3 name value pairs */
            "\xa8""msg_type"     /* : */ "\x09" // 9
            "\xac""service_name" /* : */ "\xac""service name"
            "\xa3""url"          /* : */ "\xa9""url://url",
};
// clang-format on

const char *test_name = __FILE__;
