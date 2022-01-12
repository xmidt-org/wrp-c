/* SPDX-FileCopyrightText: 2021-2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CUnit/Basic.h>
#include <cutils/xxd.h>

#include "test_common.h"

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
static int assert_wrp_int_eq(const struct wrp_int *exp,
                             const struct wrp_int *got)
{
    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_int when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_int when expecting non-NULL.");
        return -1;
    }

    if (!exp->num && !got->num) {
        return 0;
    }

    if (!exp->num && got->num) {
        CU_FAIL("We got a non-NULL wrp_int when expecting NULL.");
        return -1;
    }

    if (exp->num && !got->num) {
        CU_FAIL("We got a NULL wrp_int when expecting non-NULL.");
        return -1;
    }

    if (*exp->num != *got->num) {
        CU_FAIL("wrp_int are not equal");
        printf("wrp_int Expected: %d != Got %d\n",
               *exp->num, *got->num);
        return -1;
    }

    return 0;
}


static int assert_wrp_string_eq(const struct wrp_string *exp,
                                const struct wrp_string *got)
{
    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_string when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_string when expecting non-NULL.");
        return -1;
    }

    if (exp->len != got->len) {
        printf("Expected: %.*s\n", (int) exp->len, exp->s);
        CU_FAIL("wrp_string lengths are not equal");
        return -1;
    }

    if (0 == exp->len) {
        // printf("got->s: %p\n", got->s);
        CU_ASSERT(NULL == got->s);
        return 0;
    }

    CU_ASSERT_FATAL(NULL != exp->s);
    CU_ASSERT_FATAL(NULL != got->s);
    if (0 != memcmp(exp->s, got->s, exp->len)) {
        printf("Expected: '%.*s', Got: '%.*s'\n",
               (int) exp->len, exp->s, (int) got->len, got->s);
        CU_FAIL("wrp_string strings are not the same");
        return -1;
    }

    return 0;
}


static int assert_wrp_string_list_eq(const struct wrp_string_list *exp,
                                     const struct wrp_string_list *got)
{
    int rv = 0;

    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_string_list when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_string_list when expecting non-NULL.");
        return -1;
    }

    CU_ASSERT(exp->count == got->count);
    if (exp->count != got->count) {
        printf("Expected: %zd, Got: %zd\n", exp->count, got->count);
        return -1;
    }

    for (size_t i = 0; i < exp->count; i++) {
        rv |= assert_wrp_string_eq(&exp->list[i], &got->list[i]);
    }

    return rv;
}


static int assert_wrp_blob_eq(const struct wrp_blob *exp,
                              const struct wrp_blob *got)
{
    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_blob when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_blob when expecting non-NULL.");
        return -1;
    }

    CU_ASSERT(exp->len == got->len);
    if (exp->len != got->len) {
        printf("Expected: %zd, Got: %zd\n", exp->len, got->len);
        return -1;
    }

    if (exp->data) {
        CU_ASSERT_FATAL(NULL != got->data);
        if (0 != memcmp(exp->data, got->data, exp->len)) {
            printf("Expected: \n");
            xxd(exp->data, exp->len, stdout);
            printf("\nGot: \n");
            xxd(got->data, got->len, stdout);
            CU_FAIL("wrp_blob data is not the same");
            return -1;
        }
    }

    return 0;
}


static int assert_wrp_nvp_eq(const struct wrp_nvp *exp, const struct wrp_nvp *got)
{
    int rv = 0;

    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_nvp when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_nvp when expecting non-NULL.");
        return -1;
    }

    rv |= assert_wrp_string_eq(&exp->name, &got->name);
    rv |= assert_wrp_string_eq(&exp->value, &got->value);

    return rv;
}


static int assert_wrp_nvp_list_eq(const struct wrp_nvp_list *exp,
                                  const struct wrp_nvp_list *got)
{
    int rv = 0;

    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_nvp_list when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_nvp_list when expecting non-NULL.");
        return -1;
    }

    CU_ASSERT(exp->count == got->count);
    if (exp->count != got->count) {
        printf("Expected: %zd, Got: %zd\n", exp->count, got->count);
        return -1;
    }

    for (size_t i = 0; i < exp->count; i++) {
        rv |= assert_wrp_nvp_eq(&exp->list[i], &got->list[i]);
    }

    return rv;
}


static int assert_auth_equals(const struct wrp_auth_msg *exp,
                              const struct wrp_auth_msg *got)
{
    return assert_wrp_int_eq(&exp->status, &got->status);
}


static int assert_req_equals(const struct wrp_req_msg *exp,
                             const struct wrp_req_msg *got)
{
    int rv = 0;

    rv |= assert_wrp_string_eq(&exp->dest, &got->dest);
    rv |= assert_wrp_string_eq(&exp->source, &got->source);
    rv |= assert_wrp_blob_eq(&exp->payload, &got->payload);
    rv |= assert_wrp_string_eq(&exp->trans_id, &got->trans_id);

    rv |= assert_wrp_string_eq(&exp->accept, &got->accept);
    rv |= assert_wrp_string_eq(&exp->content_type, &got->content_type);
    rv |= assert_wrp_string_list_eq(&exp->headers, &got->headers);
    rv |= assert_wrp_nvp_list_eq(&exp->metadata, &got->metadata);
    rv |= assert_wrp_string_eq(&exp->msg_id, &got->msg_id);
    rv |= assert_wrp_string_list_eq(&exp->partner_ids, &got->partner_ids);
    rv |= assert_wrp_int_eq(&exp->rdr, &got->rdr);
    rv |= assert_wrp_string_eq(&exp->session_id, &got->session_id);
    rv |= assert_wrp_int_eq(&exp->status, &got->status);

    return rv;
}


static int assert_event_equals(const struct wrp_event_msg *exp,
                               const struct wrp_event_msg *got)
{
    int rv = 0;

    rv |= assert_wrp_string_eq(&exp->dest, &got->dest);
    rv |= assert_wrp_string_eq(&exp->source, &got->source);

    rv |= assert_wrp_string_eq(&exp->content_type, &got->content_type);
    rv |= assert_wrp_string_list_eq(&exp->headers, &got->headers);
    rv |= assert_wrp_nvp_list_eq(&exp->metadata, &got->metadata);
    rv |= assert_wrp_string_eq(&exp->msg_id, &got->msg_id);
    rv |= assert_wrp_string_list_eq(&exp->partner_ids, &got->partner_ids);
    rv |= assert_wrp_blob_eq(&exp->payload, &got->payload);
    rv |= assert_wrp_string_eq(&exp->session_id, &got->session_id);

    return rv;
}


static int assert_crud_equals(const struct wrp_crud_msg *exp,
                              const struct wrp_crud_msg *got)
{
    int rv = 0;

    rv |= assert_wrp_string_eq(&exp->dest, &got->dest);
    rv |= assert_wrp_blob_eq(&exp->payload, &got->payload);
    rv |= assert_wrp_string_eq(&exp->source, &got->source);
    rv |= assert_wrp_string_eq(&exp->trans_id, &got->trans_id);

    rv |= assert_wrp_string_eq(&exp->accept, &got->accept);
    rv |= assert_wrp_string_eq(&exp->content_type, &got->content_type);
    rv |= assert_wrp_string_list_eq(&exp->headers, &got->headers);
    rv |= assert_wrp_nvp_list_eq(&exp->metadata, &got->metadata);
    rv |= assert_wrp_string_eq(&exp->msg_id, &got->msg_id);
    rv |= assert_wrp_string_list_eq(&exp->partner_ids, &got->partner_ids);
    rv |= assert_wrp_string_eq(&exp->path, &got->path);
    rv |= assert_wrp_int_eq(&exp->rdr, &got->rdr);
    rv |= assert_wrp_string_eq(&exp->session_id, &got->session_id);
    rv |= assert_wrp_int_eq(&exp->status, &got->status);

    return rv;
}


static int assert_svc_reg_equals(const struct wrp_svc_reg_msg *exp,
                                 const struct wrp_svc_reg_msg *got)
{
    int rv = 0;

    rv |= assert_wrp_string_eq(&exp->service_name, &got->service_name);
    rv |= assert_wrp_string_eq(&exp->url, &got->url);

    return rv;
}


static int assert_wrp_equals(const wrp_msg_t *exp, const wrp_msg_t *got)
{
    int rv = 0;

    if (!exp && !got) {
        return 0;
    }

    if (!exp && got) {
        CU_FAIL("We got a non-NULL wrp_msg_t when expecting NULL.");
        return -1;
    }

    if (exp && !got) {
        CU_FAIL("We got a NULL wrp_msg_t when expecting non-NULL.");
        return -1;
    }

    CU_ASSERT_FATAL(exp->msg_type == got->msg_type);

    rv |= assert_wrp_string_eq(exp->source, got->source);
    rv |= assert_wrp_string_eq(exp->dest, got->dest);

    switch (exp->msg_type) {
        case WRP_MSG_TYPE__AUTH:
            rv |= assert_auth_equals(&exp->u.auth, &got->u.auth);
            break;
        case WRP_MSG_TYPE__REQ:
            rv |= assert_req_equals(&exp->u.req, &got->u.req);
            break;
        case WRP_MSG_TYPE__EVENT:
            rv |= assert_event_equals(&exp->u.event, &got->u.event);
            break;
        case WRP_MSG_TYPE__CREATE:
        case WRP_MSG_TYPE__RETRIEVE:
        case WRP_MSG_TYPE__UPDATE:
        case WRP_MSG_TYPE__DELETE:
            rv |= assert_crud_equals(&exp->u.crud, &got->u.crud);
            break;
        case WRP_MSG_TYPE__SVC_REG:
            rv |= assert_svc_reg_equals(&exp->u.reg, &got->u.reg);
            break;
        case WRP_MSG_TYPE__SVC_ALIVE:
            break;
        default:
            CU_FAIL("Invalid msg_type.");
            rv |= -1;
            break;
    }

    return rv;
}

static void test_wrp_to_msgpack_helper(uint8_t **got, size_t *len)
{
    WRPcode rv;

    rv = wrp_to_msgpack(&test.in, got, len);
    if (rv != test.wrp_to_msgpack_rv) {
        printf("rv: Expected: %d, Got: %d\n", test.wrp_to_msgpack_rv, rv);
    }
    CU_ASSERT_FATAL(test.wrp_to_msgpack_rv == rv);

    if (WRPE_OK == rv) {
        size_t goal_len        = test.msgpack_len;
        const char *goal_bytes = test.msgpack;
        if (test.asymetric_active) {
            goal_len   = test.asymetric_msgpack_len;
            goal_bytes = test.asymetric_msgpack;
        }

        CU_ASSERT_FATAL(NULL != *got);
        if (*len != goal_len) {
            printf("Expected: %zd, Got: %zd\n", goal_len, *len);
        }
        CU_ASSERT_FATAL(*len == goal_len);

        if (0 != memcmp(goal_bytes, *got, *len)) {
            printf("Expected:\n");
            xxd(goal_bytes, goal_len, stdout);
            printf("Got:\n");
            xxd(*got, *len, stdout);
            CU_FAIL("Buffers didn't match");
            for (size_t i = 0; i < *len; i++) {
                const uint8_t *g = *got;
                if (g[i] != (uint8_t) goal_bytes[i]) {
                    printf("difference at offset: 0x%02zx\n", i);
                }
            }
        }
    }
}

static void test_wrp_from_msgpack()
{
    wrp_msg_t *got = NULL;
    WRPcode rv;

    rv = wrp_from_msgpack(test.msgpack, test.msgpack_len, &got);
    if (rv != test.wrp_to_msgpack_rv) {
        printf("rv: Expected: %d, Got: %d\n", test.wrp_from_msgpack_rv, rv);
    }
    CU_ASSERT(test.wrp_from_msgpack_rv == rv);

    if (WRPE_OK == rv) {
        CU_ASSERT(0 == assert_wrp_equals(&test.in, got));
    }
    wrp_destroy(got);
}


static void test_wrp_to_msgpack()
{
    uint8_t *got = NULL;
    size_t len   = 0;
    uint8_t buf[1024];

    /* Use the local buffer */
    got = (uint8_t *) buf;
    len = 1024;
    test_wrp_to_msgpack_helper(&got, &len);

    /* Allocate memory */
    got = NULL;
    len = 0;
    test_wrp_to_msgpack_helper(&got, &len);

    if (got) {
        free(got);
    }
}


static void test_wrp_to_string()
{
    char *got  = NULL;
    size_t len = 0;
    WRPcode rv;

    rv = wrp_to_string(&test.in, &got, &len);

    CU_ASSERT(rv == test.wrp_to_string_rv);
    if (rv != test.wrp_to_string_rv) {
        printf("rv: Expected: %d, Got: %d\n", test.wrp_to_string_rv, rv);
    }
    if (NULL != test.string) {
        size_t exp_len = strlen(test.string);
        CU_ASSERT_FATAL(NULL != got);
        CU_ASSERT(exp_len == len);
        if (exp_len != len) {
            printf("len: Expected: %zd, Got: %zd\n", exp_len, len);
        }

        if (0 != memcmp(test.string, got, exp_len)) {
            printf("Expected:\n%s\nGot:\n%.*s\n",
                   test.string, (int) len, got);

            CU_FAIL("Strings don't match.");

            printf("Expected:\n");
            xxd(test.string, exp_len, stdout);
            printf("Got:\n");
            xxd(got, len, stdout);

            for (size_t i = 0; i < exp_len; i++) {
                if (got[i] != test.string[i]) {
                    printf("difference at offset: 0x%02zx\n", i);
                }
            }
        }
    }

    if (got) {
        free(got);
    }
}


static void add_suites(CU_pSuite *suite)
{
    *suite = CU_add_suite(test_name, NULL, NULL);
    CU_add_test(*suite, "Test wrp_from_msgpack()", test_wrp_from_msgpack);
    CU_add_test(*suite, "Test wrp_to_msgpack()  ", test_wrp_to_msgpack);
    CU_add_test(*suite, "Test wrp_to_string()  ", test_wrp_to_string);
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
