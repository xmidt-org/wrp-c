/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <stdlib.h>

#include <trower-base64/base64.h>

#include "b64.h"
#include "msgpack.h"
#include "wrp-c.h"

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
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

ssize_t wrp_struct_to_base64( const wrp_msg_t *msg, char **bytes )
{
    ssize_t data_len;
    ssize_t rv = -1;
    char *data = NULL;

    data_len = wrp_struct_to_bytes( msg, &data );
    if( 0 < data_len ) {
        size_t b64_len = 0;
        char *base64 = NULL;

        base64 = b64_encode_with_alloc( (uint8_t*) data, data_len, &b64_len );

        if( base64 ) {
            *bytes = base64;
            rv = b64_len;
        }
    }

    if( data ) {
        free( data );
    }

    return rv;
}


ssize_t wrp_base64_to_struct( const void *b64, const size_t b64_len, wrp_msg_t **msg )
{
    ssize_t rv = -1;
    size_t length = 0;
    uint8_t *bytes = NULL;

    bytes = b64_decode_with_alloc( (const uint8_t*) b64, b64_len, &length );
    if( bytes ) {
        rv = wrp_bytes_to_struct( bytes, length, msg );
        free( bytes );
    }

    return rv;
}


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */

