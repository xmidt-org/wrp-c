/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-FileCopyrightText: 2021 Weston Schmidt */
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

#endif

