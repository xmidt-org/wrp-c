/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __B64_H__
#define __B64_H__

#include <stddef.h>

#include "wrp-c.h"


/**
 *  Do the work of converting into base64 format.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the place to put the output (never NULL)
 *
 *  @return size of buffer (in bytes) on success or less then 1 on error
 */
ssize_t wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes );


/*
**
 *  Decode base64 encoded msgpack encoded data in bytes and convert to wrp struct
 *
 *  @param bytes [in]    base64 encoded msgpack encoded message in bytes
 *  @param len   [in]    length of base64 encoded message
 *  @param msg   [inout] wrp_msg_t struct initialized
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */
ssize_t wrp_base64_to_struct( const void *b64, const size_t b64_len, wrp_msg_t **msg );
#endif

