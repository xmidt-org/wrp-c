/*
 * SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <stdbool.h>

#include "../src/locator.h"

void test( int rv, const char *s, size_t len, struct locator *exp )
{
    struct locator got;
    struct locator empty;

    memset( &empty, 0, sizeof(struct locator) );
    if( NULL == exp ) {
        exp = &empty;
    }

    printf( "\n\ns = '%s', len = %zd\n", s, len );
    CU_ASSERT_FATAL( rv == string_to_locator(s, len, &got) );
    if( 0 == rv ) {
        printf( "scheme.len    = '%zd'\n", got.scheme.len );
        printf( "scheme.s      = '%.*s'\n", (int) got.scheme.len, got.scheme.s );
        printf( "authority.len = '%zd'\n", got.authority.len );
        printf( "authority.s   = '%.*s'\n", (int) got.authority.len, got.authority.s );
        printf( "service.len   = '%zd'\n", got.service.len );
        printf( "service.s     = '%.*s'\n", (int) got.service.len, got.service.s );
        printf( "app.len       = '%zd'\n", got.app.len );
        printf( "app.s         = '%.*s'\n", (int) got.app.len, got.app.s );

        CU_ASSERT( exp->scheme.s == got.scheme.s );
        CU_ASSERT( exp->scheme.len == got.scheme.len );
        CU_ASSERT( exp->authority.s == got.authority.s );
        CU_ASSERT( exp->authority.len == got.authority.len );
        CU_ASSERT( exp->service.s == got.service.s );
        CU_ASSERT( exp->service.len == got.service.len );
        CU_ASSERT( exp->app.s == got.app.s );
        CU_ASSERT( exp->app.len == got.app.len );
    }
}

void test_01(void)
{
                 /*            11111111112222222222
                  *  012345678901234567890123456789 */
    const char *v = "mac:112233445566/service/app";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4],
        .authority.len = 12,
        .service.s     = &v[17],
        .service.len   = 7,
        .app.s         = &v[25],
        .app.len       = 3,
    };

    test( 0, v, 28, &expect );
}

void test_02(void)
{
                 /*  01234567 */
    const char *v = "mac:11/s";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4],
        .authority.len = 2,
        .service.s     = &v[7],
        .service.len   = 1,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 8, &expect );
}

void test_03(void)
{
                 /*  012345678*/
    const char *v = "mac:11/s/";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4],
        .authority.len = 2,
        .service.s     = &v[7],
        .service.len   = 1,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 9, &expect );
}


void test_04(void)
{
                 /*  0123456*/
    const char *v = "mac:111";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4],
        .authority.len = 3,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 7, &expect );
}

void test_05(void)
{
                 /*  0123456*/
    const char *v = "mac:11/";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4],
        .authority.len = 2,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 7, &expect );
}

void test_06(void)
{
                 /*  0123456*/
    const char *v = "mac:/fo";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = NULL,
        .authority.len = 0,
        .service.s     = &v[5],
        .service.len   = 2,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 7, &expect );
}


void test_06_1(void)
{
                 /*  01234*/
    const char *v = "mac:/";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = NULL,
        .authority.len = 0,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 5, &expect );
}


void test_06_2(void)
{
                 /*  01234*/
    const char *v = ":/";
    struct locator expect = {
        .scheme.s      = NULL,
        .scheme.len    = 0,
        .authority.s   = NULL,
        .authority.len = 0,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 2, &expect );
}


void test_06_3(void)
{
                 /*  0123456789012*/
    const char *v = "valid/but:odd";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 5,
        .authority.s   = NULL,
        .authority.len = 0,
        .service.s     = &v[6],
        .service.len   = 7,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 13, &expect );
}


void test_06_4(void)
{
                 /*  0123456789012*/
    const char *v = "va:id/but:odd";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 2,
        .authority.s   = &v[3],
        .authority.len = 2,
        .service.s     = &v[6],
        .service.len   = 7,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 13, &expect );
}


void test_06_5(void)
{
                 /*  0123456789012*/
    const char *v = "v/a:id/:odd";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 1,
        .authority.s   = NULL,
        .authority.len = 0,
        .service.s     = &v[2],
        .service.len   = 4,
        .app.s         = &v[7],
        .app.len       = 4,
    };

    test( 0, v, 11, &expect );
}


void test_07(void)
{
                 /*  0123456*/
    const char *v = ":foo/fo";
    struct locator expect = {
        .scheme.s      = NULL,
        .scheme.len    = 0,
        .authority.s   = &v[1],
        .authority.len = 3,
        .service.s     = &v[5],
        .service.len   = 2,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 7, &expect );
}

void test_08(void)
{
                 /*  0123456*/
    const char *v = "sfoo/fo";
    struct locator expect = {
        .scheme.s      = v,
        .scheme.len    = 4,
        .authority.s   = NULL, 
        .authority.len = 0,
        .service.s     = &v[5],
        .service.len   = 2,
        .app.s         = NULL,
        .app.len       = 0,
    };


    test( 0, v, 7, &expect );
}


void test_09(void)
{
    struct locator expect = {
        .scheme.s      = NULL,
        .scheme.len    = 0,
        .authority.s   = NULL, 
        .authority.len = 0,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };


    test( 0, NULL, 7, &expect );
    test( 0, "foo", 0, &expect );
}


void test_10(void)
{
    CU_ASSERT_FATAL( -1 == string_to_locator("foo", 3, NULL ) );
}


void test_11(void)
{
                 /*            1111111111222222
                  *  01234567890123456789012345 */
    const char *v = "dns:example.com:80/foo/bar";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4], 
        .authority.len = 14,
        .service.s     = &v[19],
        .service.len   = 3,
        .app.s         = &v[23],
        .app.len       = 3,
    };

    test( 0, v, 26, &expect );
}

void test_12(void)
{
                 /*            111111111
                  *  0123456789012345678 */
    const char *v = "dns:example.com:80/";
    struct locator expect = {
        .scheme.s      = &v[0],
        .scheme.len    = 3,
        .authority.s   = &v[4], 
        .authority.len = 14,
        .service.s     = NULL,
        .service.len   = 0,
        .app.s         = NULL,
        .app.len       = 0,
    };

    test( 0, v, 19, &expect );
}


void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "locator.c tests", NULL, NULL );
    CU_add_test( *suite, "test_01", test_01 );
    CU_add_test( *suite, "test_02", test_02 );
    CU_add_test( *suite, "test_03", test_03 );
    CU_add_test( *suite, "test_04", test_04 );
    CU_add_test( *suite, "test_05", test_05 );
    CU_add_test( *suite, "test_06", test_06 );
    CU_add_test( *suite, "test_06_1", test_06_1 );
    CU_add_test( *suite, "test_06_2", test_06_2 );
    CU_add_test( *suite, "test_06_3", test_06_3 );
    CU_add_test( *suite, "test_06_4", test_06_4 );
    CU_add_test( *suite, "test_06_5", test_06_5 );
    CU_add_test( *suite, "test_07", test_07 );
    CU_add_test( *suite, "test_08", test_08 );
    CU_add_test( *suite, "test_09", test_09 );
    CU_add_test( *suite, "test_10", test_10 );
    CU_add_test( *suite, "test_11", test_11 );
    CU_add_test( *suite, "test_12", test_12 );
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( void )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;

    if( CUE_SUCCESS == CU_initialize_registry() ) {
        add_suites( &suite );

        if( NULL != suite ) {
            CU_basic_set_mode( CU_BRM_VERBOSE );
            CU_basic_run_tests();
            printf( "\n" );
            CU_basic_show_failures( CU_get_failure_list() );
            printf( "\n\n" );
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();
    }

    if( 0 != rv ) {
        return 1;
    }

    return 0;
}
