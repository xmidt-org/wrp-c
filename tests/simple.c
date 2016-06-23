/**
 *  Copyright 2010-2016 Comcast Cable Communications Management, LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <stdbool.h>

#include "../src/wrp-c.h"

struct test_to_string {
    const wrp_msg_t in;
    const char *expected;
    const ssize_t expected_length;
};

void test_to_string()
{
    const char *headers[] = { "Header 1", "Header 2", NULL };
    const struct wrp_timing_value timing[] = {
        { .name = "hop-1",
          .start.tv_sec = 123,
          .start.tv_usec = 44,
          .end.tv_sec = 124,
          .end.tv_usec = 55 },
    };

    const struct test_to_string test[] = {
        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__AUTH,
          .in.u.auth.status = 123,
          
          .expected_length = 0,
          .expected = "wrp_auth_msg {\n"
                      "    .status = 123\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__REQ,
          .in.u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .in.u.req.source = "source-address",
          .in.u.req.dest = "dest-address",
          .in.u.req.headers = NULL,
          .in.u.req.timing_values = NULL,
          .in.u.req.timing_values_count = 0,
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid    = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source              = source-address\n"
                      "    .dest                = dest-address\n"
                      "    .headers             = ''\n"
                      "    .timing_values       = ''\n"
                      "    .timing_values_count = 0\n"
                      "    .payload_size        = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__REQ,
          .in.u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .in.u.req.source = "source-address",
          .in.u.req.dest = "dest-address",
          .in.u.req.headers = (char**) headers,
          .in.u.req.timing_values = NULL,
          .in.u.req.timing_values_count = 0,
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid    = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source              = source-address\n"
                      "    .dest                = dest-address\n"
                      "    .headers             = 'Header 1, Header 2'\n"
                      "    .timing_values       = ''\n"
                      "    .timing_values_count = 0\n"
                      "    .payload_size        = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__REQ,
          .in.u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .in.u.req.source = "source-address",
          .in.u.req.dest = "dest-address",
          .in.u.req.headers = (char**) headers,
          .in.u.req.timing_values = (struct wrp_timing_value*) timing,
          .in.u.req.timing_values_count = sizeof(timing)/sizeof(struct wrp_timing_value),
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid    = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source              = source-address\n"
                      "    .dest                = dest-address\n"
                      "    .headers             = 'Header 1, Header 2'\n"
                      "    .timing_values       = \n"
                      "        hop-1: 123.000044 - 124.000055\n"
                      "    .timing_values_count = 1\n"
                      "    .payload_size        = 3\n"
                      "}\n" },
    };
    size_t i;

    for( i = 0; i < sizeof(test)/sizeof(struct test_to_string); i++ ) {
        char *out;
        ssize_t rv, expected_length;

        out = wrp_struct_to_string( &test[i].in );

        if( (NULL != out) && (0 != strcmp(out, test[i].expected)) ) {
            printf( "\n\nGot: |%s| Expected: |%s|\n\n", out, test[i].expected );
        }
        CU_ASSERT_STRING_EQUAL( out, test[i].expected );

        rv = wrp_struct_to( &test[i].in, WRP_STRING, (void**) &out );

        expected_length = test[i].expected_length;
        if( 0 == expected_length ) {
            expected_length = strlen( test[i].expected );
        }

        CU_ASSERT_STRING_EQUAL( out, test[i].expected );
        CU_ASSERT_EQUAL( rv, expected_length );
    }
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "wrp-c encoding tests", NULL, NULL );
    CU_add_test( *suite, "Test struct_to_string()", test_to_string );
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
