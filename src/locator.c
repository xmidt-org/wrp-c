/* SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <string.h>

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
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static size_t find_in_str(const char *s, char c, size_t start, size_t len)
{
    for (size_t i = start; i < len; i++) {
        if (c == s[i]) {
            return i;
        }
    }

    return len;
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
WRPcode wrp_loc_split(const char *loc, size_t len, wrp_locator_t *out)
{
    size_t colon = 0;
    size_t slash_0 = 0;
    size_t slash_1 = 0;
    WRPcode rv = WRPE_OK;

    if (!loc || !len || !out) {
        return WRPE_INVALID_ARGS;
    }

    memset(out, 0, sizeof(wrp_locator_t));

    slash_0 = find_in_str(loc, '/', 0, len);
    colon = find_in_str(loc, ':', 0, slash_0);

    slash_1 = find_in_str(loc, '/', slash_0 + 1, len);
    //printf("colon = %zd, slash_0 = %zd, slash_1 = %zd\n", colon, slash_0, slash_1);

    if (0 < colon) {
        out->scheme.s = loc;
        out->scheme.len = colon;
    } else {
        rv = WRPE_NO_SCHEME;
    }

    if (1 < (slash_0 - colon)) {
        out->authority.s = &loc[colon + 1];
        out->authority.len = slash_0 - colon - 1;
    } else if (rv == WRPE_OK) {
        rv = WRPE_NO_AUTHORITY;
    }

    if (1 < (slash_1 - slash_0)) {
        out->service.s = &loc[slash_0 + 1];
        out->service.len = slash_1 - slash_0 - 1;
    }

    if (1 < (len - slash_1)) {
        out->app.s = &loc[slash_1 + 1];
        out->app.len = len - slash_1 - 1;
    }

    return rv;
}
