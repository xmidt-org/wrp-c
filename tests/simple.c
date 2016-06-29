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
    const struct money_trace_span spans[] = {
        { .name = "hop-1",
          .start = 123000044,
          .duration = 11 },
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
          .in.u.req.include_spans = false,
          .in.u.req.spans.spans = NULL,
          .in.u.req.spans.count = 0,
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source           = source-address\n"
                      "    .dest             = dest-address\n"
                      "    .headers          = ''\n"
                      "    .include_spans    = false\n"
                      "    .spans            = ''\n"
                      "    .payload_size     = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__REQ,
          .in.u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .in.u.req.source = "source-address",
          .in.u.req.dest = "dest-address",
          .in.u.req.headers = (char**) headers,
          .in.u.req.include_spans = false,
          .in.u.req.spans.spans = NULL,
          .in.u.req.spans.count = 0,
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source           = source-address\n"
                      "    .dest             = dest-address\n"
                      "    .headers          = 'Header 1, Header 2'\n"
                      "    .include_spans    = false\n"
                      "    .spans            = ''\n"
                      "    .payload_size     = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__REQ,
          .in.u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .in.u.req.source = "source-address",
          .in.u.req.dest = "dest-address",
          .in.u.req.headers = (char**) headers,
          .in.u.req.include_spans = true,
          .in.u.req.spans.spans = (struct money_trace_span*) spans,
          .in.u.req.spans.count = sizeof(spans)/sizeof(struct money_trace_span),
          .in.u.req.payload = "123",
          .in.u.req.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_req_msg {\n"
                      "    .transaction_uuid = c07ee5e1-70be-444c-a156-097c767ad8aa\n"
                      "    .source           = source-address\n"
                      "    .dest             = dest-address\n"
                      "    .headers          = 'Header 1, Header 2'\n"
                      "    .include_spans    = true\n"
                      "    .spans            = \n"
                      "        hop-1: 123000044 - 11\n"
                      "    .payload_size     = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__EVENT,
          .in.u.event.source = "source-address",
          .in.u.event.dest = "dest-address",
          .in.u.event.headers = NULL,
          .in.u.event.payload = "123",
          .in.u.event.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_event_msg {\n"
                      "    .source           = source-address\n"
                      "    .dest             = dest-address\n"
                      "    .headers          = ''\n"
                      "    .payload_size     = 3\n"
                      "}\n" },

        /*--------------------------------------------------------------------*/
        { .in.msg_type = WRP_MSG_TYPE__EVENT,
          .in.u.event.source = "source-address",
          .in.u.event.dest = "dest-address",
          .in.u.event.headers = (char**) headers,
          .in.u.event.payload = "123",
          .in.u.event.payload_size = 3,

          .expected_length = 0,
          .expected = "wrp_event_msg {\n"
                      "    .source           = source-address\n"
                      "    .dest             = dest-address\n"
                      "    .headers          = 'Header 1, Header 2'\n"
                      "    .payload_size     = 3\n"
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

        if( (NULL != out) && (0 != strcmp(out, test[i].expected)) ) {
            printf( "\n\nGot: |%s| Expected: |%s|\n\n", out, test[i].expected );
        }
        CU_ASSERT_STRING_EQUAL( out, test[i].expected );

        if( rv != expected_length ) {
            printf( "\n\nGot: %zd Expected: %zd\n\n", rv, expected_length );
        }
        CU_ASSERT_EQUAL( rv, expected_length );
    }
}

void _internal_tva_xxd( const void *buffer, const size_t length, size_t line )
{
    const char hex[17] = "0123456789abcdef";
    const char *data = (char *) buffer;
    const char *end = &data[length];
    char output[70];

    if( (NULL == buffer) || (0 == length) ) {
        return;
    }

    while( data < end ) {
        int shiftCount;
        size_t i;
        char *text_ptr = &output[51];
        char *ptr = output;

        /* Output the '00000000:' portion */
        for (shiftCount=28; shiftCount >= 0; shiftCount -= 4) {
            *ptr++ = hex[(line >> shiftCount) & 0x0F];
        }
        *ptr++ = ':';
        *ptr++ = ' ';

        for( i = 0; i < 16; i++ ) {
            if( data < end ) {
                *ptr++ = hex[(0x0f & (*data >> 4))];
                *ptr++ = hex[(0x0f & (*data))];
                if( (' ' <= *data) && (*data <= '~') ) {
                    *text_ptr++ = *data;
                } else {
                    *text_ptr++ = '.';
                }
                data++;
            } else {
                *ptr++ = ' ';
                *ptr++ = ' ';
                *text_ptr++ = ' ';
            }
            if( 0x01 == (0x01 & i) ) {
                *ptr++ = ' ';
            }
        }
        line += 16;
        *ptr = ' ';

        *text_ptr = '\0';

        puts( output );
    }
}

void test_to_bytes()
{
	ssize_t size;
	void *bytes;
	printf("\nInside test_to_bytes()....\n");
	
	const wrp_msg_t msg = { .msg_type = WRP_MSG_TYPE__REQ,
          .u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .u.req.source = "source-address",
          .u.req.dest = "dest-address",
          .u.req.headers = NULL,
          .u.req.include_spans = false,
          .u.req.spans.spans = NULL,
          .u.req.spans.count = 0,
          .u.req.payload = "123",
          .u.req.payload_size = 3 };

	size = wrp_struct_to( &msg, WRP_BYTES, &bytes );
	/* print the encoded message */
	_internal_tva_xxd( bytes, size, 0 );
	free(bytes);
}


void test_encode_decode()
{
	ssize_t size, rv;
	void *bytes;
	wrp_msg_t *message;

	printf("\nInside test_encode_decode()....\n");
	
	const wrp_msg_t msg = { .msg_type = WRP_MSG_TYPE__REQ,
          .u.req.transaction_uuid = "c07ee5e1-70be-444c-a156-097c767ad8aa",
          .u.req.source = "source-address",
          .u.req.dest = "dest-address",
          .u.req.headers = NULL,
          .u.req.include_spans = false,
          .u.req.spans.spans = NULL,
          .u.req.spans.count = 0,
          .u.req.payload = "123",
          .u.req.payload_size = 3 };

	// msgpack encode
	size = wrp_struct_to( &msg, WRP_BYTES, &bytes );
	/* print the encoded message */
	_internal_tva_xxd( bytes, size, 0 );

	// msgpck decode
	rv = wrp_to_struct(bytes, size, WRP_BYTES, &message);
	
	CU_ASSERT_EQUAL( rv, size );
	CU_ASSERT_EQUAL( message->msg_type, msg.msg_type );
	CU_ASSERT_STRING_EQUAL( message->u.req.source, msg.u.req.source );
	CU_ASSERT_STRING_EQUAL( message->u.req.dest, msg.u.req.dest );
	CU_ASSERT_STRING_EQUAL( message->u.req.transaction_uuid, msg.u.req.transaction_uuid );
	CU_ASSERT_STRING_EQUAL( message->u.req.payload, msg.u.req.payload );
	
	printf("decoded msgType:%d\n", message->msg_type);
	printf("decoded source:%s\n", message->u.req.source);
	printf("decoded dest:%s\n", message->u.req.dest);
	printf("decoded transaction_uuid:%s\n", message->u.req.transaction_uuid);
	printf("decoded payload:%s\n", (char*)message->u.req.payload);

	wrp_free_struct(message);
	free(bytes);
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "wrp-c encoding tests", NULL, NULL );
    CU_add_test( *suite, "Test struct_to_string()", test_to_string );
    CU_add_test( *suite, "Test struct_to_bytes()", test_to_bytes );
    CU_add_test( *suite, "Test encode_decode()", test_encode_decode );
    
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
