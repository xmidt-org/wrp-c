/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wrp-c.h"

struct test_vector {
    /* Structure Version */
    const wrp_msg_t in;

    /* String Version */
    const char *string;

    /* MsgPack Version */
    const size_t msgpack_len;
    const char *msgpack;

    /* MsgPack Version if the struct -> binary data doesn't match the
     * value specified above. */
    bool asymetric_active;
    const size_t asymetric_msgpack_len;
    const char *asymetric_msgpack;

    WRPcode wrp_from_msgpack_rv;
    WRPcode wrp_to_msgpack_rv;
    WRPcode wrp_to_string_rv;
};


extern const struct test_vector test;
extern const char *test_name;
#endif
