/*
 * SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <stdbool.h>

#include "../src/wrp-c.h"

void xxd( const void *buffer, size_t length )
{
    const char hex[17] = "0123456789abcdef";
    char text[16];
    const char *data = (const char *) buffer;
    const char *end = &data[length];
    size_t line = 0;

    while( data < end ) {
        size_t i;
        char *text_ptr = text;

        /* Output the '0000000:' portion */
        putchar( hex[(0x0f & (line >> 24))] );
        putchar( hex[(0x0f & (line >> 20))] );
        putchar( hex[(0x0f & (line >> 16))] );
        putchar( hex[(0x0f & (line >> 12))] );
        putchar( hex[(0x0f & (line >>  8))] );
        putchar( hex[(0x0f & (line >>  4))] );
        putchar( hex[(0x0f & (line      ))] );
        putchar( ':' );
        putchar( ' ' );

        for( i = 0; i < 16; i++ ) {
            if( data < end ) {
                putchar( hex[(0x0f & (*data >> 4))] );
                putchar( hex[(0x0f & (*data))] );
                if( (' ' <= *data) && (*data <= '~') ) {
                    *text_ptr++ = *data;
                } else {
                    *text_ptr++ = '.';
                }
                data++;
            } else {
                putchar( ' ' );
                putchar( ' ' );
                *text_ptr++ = ' ';
            }
            if( 0x01 == (0x01 & i) ) {
                putchar( ' ' );
            }
        }
        line += 16;
        putchar( ' ' );

        for( i = 0; i < 16; i++ ) {
            putchar( text[i] );
        }
        putchar( '\n' );
    }
}


void assert_string_eq( const char *a, const char *b )
{
    CU_ASSERT_FATAL( (!a && !b) || (a && b) );
    if( a ) {
        //printf( "%s -?- %s\n", a, b );
        CU_ASSERT_STRING_EQUAL( a, b );
    }
}


void common_equal( const partners_t *ap, const partners_t *bp,
                   const headers_t *ah,  const headers_t *bh,
                   const data_t *ad,     const data_t *bd,
                   const void *apay,     const void *bpay,
                   const size_t apaylen, const size_t bpaylen )
{
    CU_ASSERT_FATAL( (!ap && !bp) || (ap && bp) );
    CU_ASSERT_FATAL( (!ah && !bh) || (ah && bh) );
    CU_ASSERT_FATAL( (!ad && !bd) || (ad && bd) );
    CU_ASSERT_FATAL( (!apay && !bpay) || (apay && bpay) );
    CU_ASSERT_FATAL( apaylen == bpaylen );

    if( ap ) {
        CU_ASSERT_FATAL( ap->count == bp->count );
        for( size_t i = 0; i < ap->count; i++ ) {
            CU_ASSERT_FATAL( NULL != ap->partner_ids[i] );
            CU_ASSERT_FATAL( NULL != bp->partner_ids[i] );
            assert_string_eq( ap->partner_ids[i], bp->partner_ids[i] );
        }
    }

    if( ah ) {
        CU_ASSERT_FATAL( ah->count == bh->count );
        for( size_t i = 0; i < ap->count; i++ ) {
            CU_ASSERT_FATAL( NULL != ah->headers[i] );
            CU_ASSERT_FATAL( NULL != bh->headers[i] );
            assert_string_eq( ah->headers[i], bh->headers[i] );
        }
    }

    if( ad ) {
        CU_ASSERT_FATAL( ad->count == bd->count );
        for( size_t i = 0; i < ad->count; i++ ) {
            assert_string_eq( ad->data_items[i].name, bd->data_items[i].name );
            assert_string_eq( ad->data_items[i].value, bd->data_items[i].value );
        }
    }

    if( apay ) {
        for( size_t i = 0; i < apaylen; i++ ) {
            CU_ASSERT( ((uint8_t*)apay)[i] == ((uint8_t*)bpay)[i] );
        }
    }
}

void equal( const wrp_msg_t *a, const wrp_msg_t *b )
{
    CU_ASSERT_FATAL( (!a && !b) || (a && b) );

    if( !a ) {
        return;
    }

    CU_ASSERT( a->msg_type == b->msg_type );

    switch( a->msg_type ) {
        case WRP_MSG_TYPE__AUTH:
            CU_ASSERT( a->u.auth.status == b->u.auth.status );
            break;

        case WRP_MSG_TYPE__REQ:
            assert_string_eq( a->u.req.transaction_uuid, b->u.req.transaction_uuid );
            assert_string_eq( a->u.req.content_type,     b->u.req.content_type );
            assert_string_eq( a->u.req.accept,           b->u.req.accept );
            assert_string_eq( a->u.req.source,           b->u.req.source );
            assert_string_eq( a->u.req.dest,             b->u.req.dest );
            CU_ASSERT( a->u.req.include_spans == b->u.req.include_spans );

            common_equal( a->u.req.partner_ids,  b->u.req.partner_ids,
                          a->u.req.headers,      b->u.req.headers,
                          a->u.req.metadata,     b->u.req.metadata,
                          a->u.req.payload,      b->u.req.payload,
                          a->u.req.payload_size, b->u.req.payload_size );
            break;

        case WRP_MSG_TYPE__EVENT:
            assert_string_eq( a->u.event.content_type,     b->u.event.content_type );
            assert_string_eq( a->u.event.source,           b->u.event.source );
            assert_string_eq( a->u.event.dest,             b->u.event.dest );

            common_equal( a->u.event.partner_ids,  b->u.event.partner_ids,
                          a->u.event.headers,      b->u.event.headers,
                          a->u.event.metadata,     b->u.event.metadata,
                          a->u.event.payload,      b->u.event.payload,
                          a->u.event.payload_size, b->u.event.payload_size );
            break;

        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETREIVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            assert_string_eq( a->u.crud.transaction_uuid, b->u.crud.transaction_uuid );
            assert_string_eq( a->u.crud.content_type,     b->u.crud.content_type );
            assert_string_eq( a->u.crud.accept,           b->u.crud.accept );
            assert_string_eq( a->u.crud.source,           b->u.crud.source );
            assert_string_eq( a->u.crud.dest,             b->u.crud.dest );
            assert_string_eq( a->u.crud.path,             b->u.crud.path );
            CU_ASSERT( a->u.crud.status == b->u.crud.status );
            CU_ASSERT( a->u.crud.rdr == b->u.crud.rdr );
            CU_ASSERT( a->u.crud.include_spans == b->u.crud.include_spans );

            common_equal( a->u.crud.partner_ids,  b->u.crud.partner_ids,
                          a->u.crud.headers,      b->u.crud.headers,
                          a->u.crud.metadata,     b->u.crud.metadata,
                          a->u.crud.payload,      b->u.crud.payload,
                          a->u.crud.payload_size, b->u.crud.payload_size );
            break;

        case WRP_MSG_TYPE__SVC_REGISTRATION:
            CU_ASSERT( a->u.reg.service_name == b->u.reg.service_name );
            CU_ASSERT( a->u.reg.url == b->u.reg.url );
            break;

        default:
            break;
    }
}

void run_test_to_bytes( const char *bytes, ssize_t len, const wrp_msg_t *s )
{
    char *out;
    ssize_t out_len;

    out_len = wrp_struct_to( s, WRP_BYTES, (void**) &out );
    CU_ASSERT_FATAL( out_len == len );

    if( 0 < len ) {
        for( ssize_t i = 0; i < len; i++ ) {
            if( out[i] != bytes[i] ) {
                printf( "%zd: expected[%zd] = 0x%02x, got[%zd] = 0x%02x\n", i, i,
                        bytes[i], i, out[i] );
            }
            CU_ASSERT( bytes[i] == out[i] );
        }
#if 0
    printf( "\n\nGot\n" );
    xxd( out, out_len );
    printf( "\n\nExpect\n" );
    xxd( bytes, len );
#endif
    }

    if( NULL != out ) {
        free( out );
    }
}

void run_test_to_struct( const char *bytes, ssize_t len, const wrp_msg_t *s )
{
    wrp_msg_t *msg = NULL;
    ssize_t back_rv;

    back_rv = wrp_to_struct( bytes, len, WRP_BYTES, &msg );
    CU_ASSERT( back_rv == len );

    equal( msg, s );

    wrp_free_struct( msg );
}


void test_00()  /* Simple boudary tests */
{
    const wrp_msg_t in = { .msg_type = (enum wrp_msg_type) 900 };
    void *out = NULL;

    CU_ASSERT( -1 == wrp_struct_to(&in, WRP_BYTES, &out) );
    CU_ASSERT( NULL == out );

    CU_ASSERT( -1 == wrp_struct_to(NULL, WRP_BYTES, &out) );
    CU_ASSERT( NULL == out );
}

/*----------------------------------------------------------------------------*/
/*                                Event Tests                                 */
/*----------------------------------------------------------------------------*/

void test_event_00()
{
    const char bytes[] = "\x84"                                            // map, size 4
                         "\xa8""msg_type" "\x04"                           // msg_type: 4
                         "\xa6""source"   "\xb8""mac:112233445566/example"
                         "\xa4""dest"     "\xb8""event:identifier/ignored"
                         "\xa7""payload"  "\xc4""\x00";                    // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    const wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__EVENT,
                          .u.event.source = "mac:112233445566/example",
                          .u.event.dest = "event:identifier/ignored",
                          .u.event.payload = NULL,
                          .u.event.payload_size = 0,
                          .u.event.metadata = NULL,
                          .u.event.headers = NULL,
                          .u.event.partner_ids = NULL,
                          .u.event.content_type = NULL,
                        };

    run_test_to_struct( bytes, bytes_len, &s );
    run_test_to_bytes( bytes, bytes_len, &s );
}

void test_event_01()    // alternate form of payload
{
    const char bytes[] = "\x84"                                            // map, size 4
                         "\xa8""msg_type" "\x04"                           // msg_type: 4
                         "\xa6""source"   "\xb8""mac:112233445566/example"
                         "\xa4""dest"     "\xb8""event:identifier/ignored"
                         "\xa7""payload"  "\xc4""\x00";                    // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    const wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__EVENT,
                          .u.event.source = "mac:112233445566/example",
                          .u.event.dest = "event:identifier/ignored",
                          .u.event.payload = "not null",
                          .u.event.payload_size = 0,
                          .u.event.metadata = NULL,
                          .u.event.headers = NULL,
                          .u.event.partner_ids = NULL,
                          .u.event.content_type = NULL,
                        };

    run_test_to_bytes( bytes, bytes_len, &s );
}

void test_event_02()    // alternate form of payload
{
    const char bytes[] = "\x84"                                            // map, size 4
                         "\xa8""msg_type" "\x04"                           // msg_type: 4
                         "\xa6""source"   "\xb8""mac:112233445566/example"
                         "\xa4""dest"     "\xb8""event:identifier/ignored"
                         "\xa7""payload"  "\xc4""\x00";                    // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    const wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__EVENT,
                          .u.event.source = "mac:112233445566/example",
                          .u.event.dest = "event:identifier/ignored",
                          .u.event.payload = NULL,
                          .u.event.payload_size = 123,
                          .u.event.metadata = NULL,
                          .u.event.headers = NULL,
                          .u.event.partner_ids = NULL,
                          .u.event.content_type = NULL,
                        };

    run_test_to_bytes( bytes, bytes_len, &s );
}

void test_event_03()    // alternate form of payload
{
    const wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__EVENT,
                          .u.event.source = NULL,
                          .u.event.dest = "event:identifier/ignored",
                          .u.event.payload = NULL,
                          .u.event.payload_size = 123,
                          .u.event.metadata = NULL,
                          .u.event.headers = NULL,
                          .u.event.partner_ids = NULL,
                          .u.event.content_type = NULL,
                        };

    run_test_to_bytes( NULL, -1, &s );
}

void test_req_00()    // check bool values
{
    const char bytes[] = "\x86"                                            // map, size 6
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xa7""payload"          "\xc4""\x00";            // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    const wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__REQ,
                          .u.req.transaction_uuid = "f8013ad2-438a-46dd-8b62-45a3e1f95561",
                          .u.req.source = "mac:112233445566/example",
                          .u.req.dest = "dns:identifier/ignored",
                          .u.req.payload = NULL,
                          .u.req.payload_size = 0,
                          .u.req.metadata = NULL,
                          .u.req.headers = NULL,
                          .u.req.partner_ids = NULL,
                          .u.req.content_type = NULL,
                          .u.req.include_spans = true,
                        };

    run_test_to_bytes( bytes, bytes_len, &s );
    run_test_to_struct( bytes, bytes_len, &s );
}


void test_req_01()    // check bool values in metadata
{
    const char bytes[] = "\x87"                                            // map, size 8
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xa8""metadata"         "\x85"
                             "\xa3""foo"          "\xc3"                   // true
                             "\xa3""bar"          "\xc2"                   // false
                             "\xa3""num"          "\xd3\x00\x00\x00\x00\x00\x00\x00\x0c" // 12
                             "\xa3""big"          "\xcf\x00\x00\x00\x00\x00\x00\x00\x0c" // 12
                             "\xa3""neg"          "\xd3\xff\xff\xff\xff\xff\xff\xff\xf4" // -12
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xa7""payload"          "\xc4""\x00";            // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    struct data d[5] = {
        { .name = "foo", .value = "true" },
        { .name = "bar", .value = "false" },
        { .name = "num", .value = "12" },
        { .name = "big", .value = "12" },
        { .name = "neg", .value = "-12" },
    };
    data_t metadata = { .count = 5, .data_items = d };
    wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__REQ,
                          .u.req.transaction_uuid = "f8013ad2-438a-46dd-8b62-45a3e1f95561",
                          .u.req.source = "mac:112233445566/example",
                          .u.req.dest = "dns:identifier/ignored",
                          .u.req.payload = NULL,
                          .u.req.payload_size = 0,
                          .u.req.metadata = NULL,
                          .u.req.headers = NULL,
                          .u.req.partner_ids = NULL,
                          .u.req.content_type = NULL,
                          .u.req.include_spans = true,
                        };

    s.u.req.metadata = &metadata;

    run_test_to_struct( bytes, bytes_len, &s );
}


void test_req_02()    // check ignored fields
{
    const char bytes[] = "\x8d"                                            // map, size 13
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xad""ignored_field"    "\xc3"                   // bool ignored
                         "\xad""ignored_field"    "\x03"                   // int ignored
                         "\xad""ignored_field"    "\xa3""doh"              // str ignored
                         "\xad""ignored_field"    "\x90"                   // arr ignored
                         "\xad""ignored_field"    "\xc0"                   // nil ignored
                         "\xad""ignored_field"    "\x80"                   // map ignored
                         "\xad""ignored_field"    "\xc4\x00"               // bin ignored
                         "\xa7""payload"          "\xc4\x00";              // empty payload
    ssize_t bytes_len = sizeof(bytes) - 1;

    wrp_msg_t s = { .msg_type = WRP_MSG_TYPE__REQ,
                          .u.req.transaction_uuid = "f8013ad2-438a-46dd-8b62-45a3e1f95561",
                          .u.req.source = "mac:112233445566/example",
                          .u.req.dest = "dns:identifier/ignored",
                          .u.req.payload = NULL,
                          .u.req.payload_size = 0,
                          .u.req.metadata = NULL,
                          .u.req.headers = NULL,
                          .u.req.partner_ids = NULL,
                          .u.req.content_type = NULL,
                          .u.req.include_spans = true,
                        };

    run_test_to_struct( bytes, bytes_len, &s );
}


void test_req_03()    // check duplicate fields
{
    const char bytes[] = "\x85"                                            // map, size 7
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xa7""payload"          "\xc4\x00"               // empty payload
                         "\xa7""payload"          "\xc4\x00";              // dup
    ssize_t len = sizeof(bytes) - 1;
    wrp_msg_t *msg = NULL;

    CU_ASSERT( -1 == wrp_to_struct(bytes, len, WRP_BYTES, &msg) );
}

void test_req_04()    // check duplicate fields
{
    const char bytes[] = "\x87"                                            // map, size 7
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xa7""payload"          "\xc4\x00"               // empty payload
                         "\xad""include_spans"    "\xc2";                  // dup
    ssize_t len = sizeof(bytes) - 1;
    wrp_msg_t *msg = NULL;

    CU_ASSERT( -1 == wrp_to_struct(bytes, len, WRP_BYTES, &msg) );
}

void test_req_05()    // check duplicate fields
{
    const char bytes[] = "\x87"                                            // map, size 7
                         "\xa8""msg_type"         "\x03"                   // msg_type: 3
                         "\xa6""source"           "\xb8""mac:112233445566/example"
                         "\xa4""dest"             "\xb6""dns:identifier/ignored"
                         "\xb0""transaction_uuid" "\xd9\x24""f8013ad2-438a-46dd-8b62-45a3e1f95561"
                         "\xad""include_spans"    "\xc3"
                         "\xa7""payload"          "\xc4\x00"               // empty payload
                         "\xa4""dest"             "\xa4zzzz";              // dup
    ssize_t len = sizeof(bytes) - 1;
    wrp_msg_t *msg = NULL;

    CU_ASSERT( -1 == wrp_to_struct(bytes, len, WRP_BYTES, &msg) );
}


void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "utils.c tests", NULL, NULL );
    CU_add_test( *suite, "test 00", test_00 );
    CU_add_test( *suite, "event 00", test_event_00 );
    CU_add_test( *suite, "event 01", test_event_01 );
    CU_add_test( *suite, "event 02", test_event_02 );
    CU_add_test( *suite, "event 03", test_event_03 );
    CU_add_test( *suite, "req 00", test_req_00 );
    CU_add_test( *suite, "req 01", test_req_01 );
    CU_add_test( *suite, "req 02", test_req_02 );
    CU_add_test( *suite, "req 03", test_req_03 );
    CU_add_test( *suite, "req 04", test_req_04 );
    CU_add_test( *suite, "req 05", test_req_05 );
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
