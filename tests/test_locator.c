/*
 * SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <CUnit/Basic.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/xxd.h>

#include "wrp-c.h"

void test(WRPcode rv, const char *in, const char *exp)
{
    wrp_locator_t got;
    WRPcode got_rv;
    size_t len = 0;

    got_rv = wrp_loc_split(in, strlen(in), &got);
    if (rv != got_rv) {
        printf("Got: %d, Expected: %d, In: %s\n", got_rv, rv, in);
    }
    CU_ASSERT_FATAL(rv == got_rv);

    if (WRPE_OK == rv) {
        char *out = NULL;

        wrp_loc_to_string(&got, &out, &len);

        if (NULL == out) {
            printf("Test: %s\n", in);
        }
        CU_ASSERT_FATAL(NULL != out);

        if (strlen(exp) != len) {
            printf("Got:\n%.*s\nExpected:\n%s\n", (int) len, out, exp);
        }
        CU_ASSERT_FATAL(strlen(exp) == len);

        if (memcmp(exp, out, len)) {
            printf("Got:\n%.*s\nExpected:\n%s\n", (int) len, out, exp);
            printf("Got:\n");
            xxd(out, len, stdout);
            printf("Expected:\n");
            xxd(exp, strlen(exp), stdout);
            CU_FAIL("Strings are not equal");
        }

        if (out) free(out);
    }
}

void test_00(void)
{
    // clang-format off
    struct test_vector {
        WRPcode rv;
        const char *in;
        const char *exp;
    } tests[] = {
        { WRPE_OK, "mac:112233445566/myService/myApp", "wrp_locator_t {\n"
                                                       "    .scheme    = 'mac'\n"
                                                       "    .authority = '112233445566'\n"
                                                       "    .service   = 'myService'\n"
                                                       "    .app       = 'myApp'\n"
                                                       "}\n" },
        { WRPE_OK, "mac:11/s", "wrp_locator_t {\n"
                               "    .scheme    = 'mac'\n"
                               "    .authority = '11'\n"
                               "    .service   = 's'\n"
                               "    .app       = ''\n"
                               "}\n" },
        { WRPE_OK, "valid:but/quite:odd", "wrp_locator_t {\n"
                                          "    .scheme    = 'valid'\n"
                                          "    .authority = 'but'\n"
                                          "    .service   = 'quite:odd'\n"
                                          "    .app       = ''\n"
                                          "}\n" },
        { WRPE_OK, "va:lid/:but/o:dd/stuff", "wrp_locator_t {\n"
                                             "    .scheme    = 'va'\n"
                                             "    .authority = 'lid'\n"
                                             "    .service   = ':but'\n"
                                             "    .app       = 'o:dd/stuff'\n"
                                             "}\n" },
        { WRPE_OK, "dns:example.com:80/foo/bar/stuff", "wrp_locator_t {\n"
                                                       "    .scheme    = 'dns'\n"
                                                       "    .authority = 'example.com:80'\n"
                                                       "    .service   = 'foo'\n"
                                                       "    .app       = 'bar/stuff'\n"
                                                       "}\n" },
        { WRPE_OK, "dns:example.com:80/", "wrp_locator_t {\n"
                                          "    .scheme    = 'dns'\n"
                                          "    .authority = 'example.com:80'\n"
                                          "    .service   = ''\n"
                                          "    .app       = ''\n"
                                          "}\n" },
        { WRPE_OK, "mac:111", "wrp_locator_t {\n"
                              "    .scheme    = 'mac'\n"
                              "    .authority = '111'\n"
                              "    .service   = ''\n"
                              "    .app       = ''\n"
                              "}\n" },
        { WRPE_OK, "mac:fo", "wrp_locator_t {\n"
                             "    .scheme    = 'mac'\n"
                             "    .authority = 'fo'\n"
                             "    .service   = ''\n"
                             "    .app       = ''\n"
                             "}\n" },
        { WRPE_NO_AUTHORITY, "mac:/", NULL },
        { WRPE_NO_SCHEME,    ":/", NULL },
        { WRPE_NO_SCHEME,    ":foo/fo", NULL },
        { WRPE_NO_AUTHORITY, "sfoo/fo", NULL },
    };
    // clang-format on

    for (size_t i = 0; i < sizeof(tests) / sizeof(struct test_vector); i++) {
        test(tests[i].rv, tests[i].in, tests[i].exp);
    }
}


void test_01(void)
{
    wrp_locator_t loc;
    char *out = NULL;

    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split(NULL, 0, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split("foo", 0, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split("foo", 3, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split("foo", 0, &loc));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split(NULL, 3, &loc));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_split(NULL, 3, NULL));

    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_to_string(NULL, NULL, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_to_string(&loc, NULL, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_loc_to_string(NULL, &out, NULL));


    CU_ASSERT(WRPE_OK == wrp_loc_split("mac:11/foo", 10, &loc));
    CU_ASSERT(WRPE_OK == wrp_loc_to_string(&loc, &out, NULL));
    CU_ASSERT(NULL != out);

    free(out);
}

void add_suites(CU_pSuite *suite)
{
    *suite = CU_add_suite("locator.c tests", NULL, NULL);
    CU_add_test(*suite, "test_00", test_00);
    CU_add_test(*suite, "test_01", test_01);
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main(void)
{
    unsigned rv     = 1;
    CU_pSuite suite = NULL;

    if (CUE_SUCCESS == CU_initialize_registry()) {
        add_suites(&suite);

        if (NULL != suite) {
            CU_basic_set_mode(CU_BRM_VERBOSE);
            CU_basic_run_tests();
            printf("\n");
            CU_basic_show_failures(CU_get_failure_list());
            printf("\n\n");
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();
    }

    if (0 != rv) {
        return 1;
    }

    return 0;
}
