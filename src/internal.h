/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include <stddef.h>
#include <stdint.h>

#include <mpack.h>

#include "wrp-c.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define REQUIRED 0
#define OPTIONAL 1
#define INTERNAL_SIGNATURE 0x777270

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
struct wrp_internal {
    int sig;
    mpack_tree_t tree;

    /* The list of things to free */
    void *partner_ids;
    void *metadata;
    void *headers;

    wrp_msg_t msg;
};


/**
 * A simple map function for error codes.
 */
WRPcode map_mpack_err(mpack_error_t err);

#endif
