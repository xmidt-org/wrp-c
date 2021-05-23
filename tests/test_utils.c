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

#include "../src/utils.h"


void test_wrp_strdup()
{
    char *tmp;
    char *empty = "e";

    CU_ASSERT( NULL == wrp_strdup( NULL ) );

    tmp = wrp_strdup( &empty[1] );
    CU_ASSERT( NULL != tmp );
    CU_ASSERT( '\0' == *tmp );
    free( tmp );

    tmp = wrp_strdup( "asdf" );
    CU_ASSERT( NULL != tmp );
    CU_ASSERT_STRING_EQUAL( tmp, "asdf" );
    free( tmp );
}


void test_wrp_strndup()
{
    char *tmp;
    char *empty = "e";
    CU_ASSERT( NULL == wrp_strndup( NULL, 0 ) );

    tmp = wrp_strndup( &empty[1], 5 );
    CU_ASSERT( NULL != tmp );
    CU_ASSERT( '\0' == *tmp );
    free( tmp );

    tmp = wrp_strndup( &empty[1], 0 );
    CU_ASSERT( NULL == tmp );

    tmp = wrp_strndup( "foo", 0 );
    CU_ASSERT( NULL == tmp );

    tmp = wrp_strndup( "asdf", 12 );
    CU_ASSERT( NULL != tmp );
    CU_ASSERT_STRING_EQUAL( tmp, "asdf" );
    free( tmp );

    tmp = wrp_strndup( "asdf", 2 );
    CU_ASSERT( NULL != tmp );
    CU_ASSERT_STRING_EQUAL( tmp, "as" );
    free( tmp );
}


void test_mvaprintf(void)
{
    char *got;
    size_t len;

    got = maprintf( "Hello, %s.", "world" );
    CU_ASSERT_FATAL( NULL != got );
    CU_ASSERT_STRING_EQUAL( got, "Hello, world." );
    free( got );

    got = mlaprintf( &len, "Answer: %d", 42 );
    CU_ASSERT_FATAL( NULL != got );
    CU_ASSERT( 10 == len );
    CU_ASSERT_STRING_EQUAL( got, "Answer: 42" );
    free( got );
}


void test_append(void)
{
    char *buf;
    char *p;

    buf = calloc( 9, sizeof(char) );
    CU_ASSERT_FATAL( NULL != buf );

    memset( buf, '-', 9 );

    p = wrp_append( buf, "1234" );
    CU_ASSERT_STRING_EQUAL( buf, "1234" );

    p = wrp_append( p, "abcd" );
    CU_ASSERT_STRING_EQUAL( buf, "1234abcd" );

    free( buf );
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "utils.c tests", NULL, NULL );
    CU_add_test( *suite, "wrp_append() Tests", test_append );
    CU_add_test( *suite, "wrp_strdup() Tests", test_wrp_strdup );
    CU_add_test( *suite, "wrp_strndup() Tests", test_wrp_strndup );
    CU_add_test( *suite, "mvaprintf() Tests", test_mvaprintf );
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
