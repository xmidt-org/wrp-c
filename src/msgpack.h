/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __MSGPACK_H__
#define __MSGPACK_H__

#include <stddef.h>

#include "wrp-c.h"


/**
 *  Do the work of converting into msgpack binary format.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the place to put the output (never NULL)
 *
 *  @return size of buffer (in bytes) on success or less then 1 on error
 */
ssize_t wrp_struct_to_bytes( const wrp_msg_t *msg, char **bytes );


/**
 *  Decode msgpack encoded data in bytes and convert to wrp struct
 *
 *  @param bytes [in]  msgpack encoded message in bytes
 *  @param length [in] length of encoded message
 *  @param msg_ptr [inout] wrp_msg_t struct initialized using bytes
 *
 *  @return the number of bytes 'consumed' by making this transformation if
 *          successful, less than 1 otherwise
 */
ssize_t wrp_bytes_to_struct( const void *bytes, size_t length, wrp_msg_t **msg_ptr );
#endif

