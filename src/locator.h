/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __LOCATOR_H__
#define __LOCATOR_H__

#include <stddef.h>
#include <stdint.h>

struct wrp_string {
    const char *s;
    size_t len;
};

struct locator {
    struct wrp_string scheme;
    struct wrp_string authority;
    struct wrp_string service;
    struct wrp_string app;
};


/**
 *  Populates the locator structure based on the presented string.
 *
 *  @param s   the string to examine
 *  @param len the length of the string
 *  @param l   the locator structure to populate
 *
 *  @return 0 if sucessful, error otherwise
 */
int string_to_locator( const char *s, size_t len, struct locator *l );

#endif

