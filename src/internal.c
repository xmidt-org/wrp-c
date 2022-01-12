/* SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>

#include "internal.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
WRPcode map_mpack_err(mpack_error_t err)
{
    WRPcode rv = WRPE_OK;

    switch (err) {
        case mpack_ok:
            rv = WRPE_OK;
            break;

        case mpack_error_invalid:
        case mpack_error_unsupported:
            rv = WRPE_NOT_MSGPACK_FORMAT;
            break;

        case mpack_error_type:
        case mpack_error_data:
            rv = WRPE_NOT_A_WRP_MSG;
            break;

        case mpack_error_too_big:
            rv = WRPE_MSG_TOO_BIG;
            break;

        case mpack_error_memory:
            rv = WRPE_OUT_OF_MEMORY;
            break;

        default:
            rv = WRPE_OTHER_ERROR;
            break;
    }

    return rv;
}
