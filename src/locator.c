/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-FileCopyrightText: 2021 Weston Schmidt */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "locator.h"

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
size_t find_in_str( const char *s, char c, size_t start, size_t len );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/* See locator.h for details. */
int string_to_locator( const char *s, size_t len, struct locator *l )
{
    size_t colon = 0;
    size_t slash_0 = 0;
    size_t slash_1 = 0;

    if( !l ) {
        return -1;
    }

    memset( l, 0, sizeof(struct locator) );

    if( !s || !len ) {
        return 0;
    }

    slash_0 = find_in_str( s, '/', 0, len );
    colon = find_in_str( s, ':', 0, slash_0 );

    slash_1 = find_in_str( s, '/', slash_0 + 1, len );
    printf( "colon = %zd, slash_0 = %zd, slash_1 = %zd\n", colon, slash_0, slash_1 );

    if( 0 < colon ) {
        l->scheme.s = s;
        l->scheme.len = colon;
    }


    if( 1 < (slash_0 - colon) ) {
        l->authority.s = &s[colon + 1];
        l->authority.len = slash_0 - colon - 1;
    }

    if( 1 < (slash_1 - slash_0) ) {
        l->service.s = &s[slash_0 + 1];
        l->service.len = slash_1 - slash_0 - 1;
    }

    if( 1 < (len - slash_1) ) {
        l->app.s = &s[slash_1 + 1];
        l->app.len = len - slash_1 - 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */

size_t find_in_str( const char *s, char c, size_t start, size_t len )
{
    for( size_t i = start; i < len; i++ ) {
        if( c == s[i] ) {
            return i;
        }
    }

    return len;
}
