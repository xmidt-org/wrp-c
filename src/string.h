/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __WRP_STRING_H__
#define __WRP_STRING_H__

#include <stdlib.h>

#include "wrp-c.h"

/**
 *  Split out the different types to different functions.
 *
 *  @param msg   [in]  the message to convert
 *  @param bytes [out] the output of the conversion
 *
 *  @return the number of bytes in the string or less then 1 on error
 */
ssize_t internal_struct_to_string( const wrp_msg_t *msg, char **bytes );

#endif
