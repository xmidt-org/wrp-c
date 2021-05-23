/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdarg.h>
#include <stddef.h>

/* Versions of printf() that provide an on demand allocated buffer
 * that fits the data. */
char* maprintf( const char *format, ... );
char* mvaprintf( const char *format, va_list args );

/* Versions of printf() that provide an on demand allocated buffer that
 * fits the data.  The strlen() value of the string is returned in len if
 * it is not NULL. */
char* mlaprintf( size_t *len, const char *format, ... );
char* mlvaprintf( size_t *len, const char *format, va_list args );


/**
 *  @brief Helper function that copies a portion of a string defined by pointers
 *
 *  @param s the start of the string to copy
 *  @param e the last character of the string to copy
 *
 *  @return the allocated buffer with the substring
 */
char* strdupptr( const char *s, const char *e );

/**
 * 'standard' but often missing functions
 */
char* wrp_strdup( const char *s );
char* wrp_strndup( const char *s, size_t n );

/**
 *  Appends 'src' to dest and includes the trailing '\0'.
 *
 *  @note: The caller must ensure there is enough buffer space in dest.
 *  @note: Don't overlap buffers.
 *
 *  @param dest the destination to append to
 *  @param src  the string to append
 *
 *  @return the pointer to the '\0' at the end of what was appended.
 */
char* wrp_append( char *dest, const char *src );

#endif

