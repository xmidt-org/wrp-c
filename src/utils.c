/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-FileCopyrightText: 2021 Weston Schmidt */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

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
char* maprintf( const char *format, ... )
{
    va_list args;
    char *buf = NULL;

    va_start( args, format );
    buf = mvaprintf( format, args );
    va_end( args );

    return buf;
}


char* mvaprintf( const char *format, va_list args )
{
    return mlvaprintf( NULL, format, args );
}


char* mlaprintf( size_t *len, const char *format, ... )
{
    va_list args;
    char *buf = NULL;

    va_start( args, format );
    buf = mlvaprintf( len, format, args );
    va_end( args );

    return buf;
}


char* mlvaprintf( size_t *len, const char *format, va_list args )
{
    va_list copy;
    char *buf = NULL;
    int l;

    va_copy( copy, args );
    l = vsnprintf( NULL, 0, format, copy );
    if( 0 < l ) {

        buf = malloc( l + 1 );
        if( buf ) {
            int rv = vsprintf( buf, format, args );

            if( rv != l ) {
                free( buf );
                buf = NULL;
                l = 0;
            }
        } else {
            l = 0;
        }
    }

    if( len ) {
        *len = (size_t) l;
    }

    return buf;
}


char* strdupptr( const char *s, const char *e )
{
    if( s == e ) {
        return NULL;
    }

    return wrp_strndup( s, (size_t)(((uintptr_t)e) - ((uintptr_t)s)) );
}


char* wrp_strdup( const char *s )
{
    char *rv = NULL;

    if( s ) {
        size_t len;

        len = strlen( s ) + 1;
        rv = malloc( len );
        if( rv ) {
            memcpy( rv, s, len );
        }
    }

    return rv;
}


char* wrp_strndup( const char *s, size_t n )
{
    char *rv = NULL;

    if( (NULL != s) && (0 < n) ) {
        size_t len;

        len = strlen( s );
        if( n < len ) {
            len = n;
        }

        rv = malloc( len + 1 ); /* +1 for trailing '\0' */
        if (rv) {
            memcpy( rv, s, len );
            rv[len] = '\0';
        }
    }

    return rv;
}
