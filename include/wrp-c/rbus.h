/* SPDX-FileCopyrightText: 2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __WRPC_RBUS_H__
#define __WRPC_RBUS_H__

#include "wrp-c.h"
#include <rbus/rbus.h>
#include <stddef.h>
#include <stdint.h>

/**
 *  Converts a wrp structure to an rbus encoded form.
 *
 *  @param src  the message to encode
 *  @param dest If *dest != NULL then encode into the specified buffer.
 *              If *dest == NULL then the function will allocate a buffer and
 *              return a pointer to it here.
 *  @param len  The dest buffer length if provided, and the number of valid
 *              encoded byte in dest.
 *
 *  @retval WRPE_OK
 *  @retval WRPE_INVALID_ARGS
 *  @retval WRPE_NOT_A_WRP_MSG
 *  @retval WRPE_MSG_TOO_BIG
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_OTHER_ERROR
 */
WRPcode wrp_to_rbus(const wrp_msg_t *src, rbusObject_t *dest);


/**
 *  Converts an rbus encoded message into the wrp c structure.
 *
 *  @note All data is copied from the rbus objects because it does not
 *        provide a way to reference the information directly and safely.
 *
 *  @param src  the rbusObject_t object
 *  @param dest the resulting object (must be released by caller)
 *
 *  @retval WRPE_OK
 *  @retval WRPE_INVALID_ARGS
 *  @retval WRPE_NOT_MSGPACK_FORMAT
 *  @retval WRPE_NOT_A_WRP_MSG
 *  @retval WRPE_MSG_TOO_BIG
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_OTHER_ERROR
 */
WRPcode wrp_from_rbus(const rbusObject_t src, wrp_msg_t **dest);
#endif
