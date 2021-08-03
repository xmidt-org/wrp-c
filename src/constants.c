/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "constants.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define MAKE_TOKEN(str)                  \
    {                                    \
        .s = str, .len = sizeof(str) - 1 \
    }

// clang-format off
const struct wrp_token WRP_MSG_TYPE = MAKE_TOKEN( "msg_type" );
const struct wrp_token WRP_SOURCE__ = MAKE_TOKEN( "source" );
const struct wrp_token WRP_DEST____ = MAKE_TOKEN( "dest" );
const struct wrp_token WRP_CT______ = MAKE_TOKEN( "content_type" );
const struct wrp_token WRP_PARTNERS = MAKE_TOKEN( "partner_ids" );
const struct wrp_token WRP_PAYLOAD_ = MAKE_TOKEN( "payload" );
const struct wrp_token WRP_METADATA = MAKE_TOKEN( "metadata" );
const struct wrp_token WRP_TRANS_ID = MAKE_TOKEN( "transaction_uuid" );
const struct wrp_token WRP_ACCEPT__ = MAKE_TOKEN( "accept" );
const struct wrp_token WRP_STATUS__ = MAKE_TOKEN( "status" );
const struct wrp_token WRP_RDR_____ = MAKE_TOKEN( "rdr" );
const struct wrp_token WRP_PATH____ = MAKE_TOKEN( "path" );
const struct wrp_token WRP_SN______ = MAKE_TOKEN( "service_name" );
const struct wrp_token WRP_URL_____ = MAKE_TOKEN( "url" );
// clang-format on
