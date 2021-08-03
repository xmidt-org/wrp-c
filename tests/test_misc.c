/*
 * SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC
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
#include <mpack.h>

#include "wrp-c.h"

extern WRPcode map_mpack_err(mpack_error_t err);

void test_01(void)
{
    wrp_msg_t msg;
    char *out = NULL;

    memset(&msg, 0, sizeof(wrp_msg_t));
    msg.msg_type = WRP_MSG_TYPE__SVC_ALIVE;

    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_string(NULL, NULL, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_string(&msg, NULL, NULL));

    CU_ASSERT(WRPE_OK == wrp_to_string(&msg, &out, NULL));
    CU_ASSERT(NULL != out);

    free(out);
}

void test_02(void)
{
    CU_ASSERT(WRPE_MSG_TOO_BIG == map_mpack_err(mpack_error_too_big));
    CU_ASSERT(WRPE_OUT_OF_MEMORY == map_mpack_err(mpack_error_memory));
    CU_ASSERT(WRPE_OTHER_ERROR == map_mpack_err((mpack_error_t)100));
}

void test_03(void)
{
    wrp_msg_t msg;
    uint8_t *buf = NULL;
    size_t len = 0;

    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_msgpack(NULL, NULL, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_msgpack(&msg, NULL, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_msgpack(&msg, &buf, NULL));
    CU_ASSERT(WRPE_INVALID_ARGS == wrp_to_msgpack(NULL, &buf, &len));
}

void add_suites(CU_pSuite *suite)
{
    *suite = CU_add_suite("misc tests", NULL, NULL);
    CU_add_test(*suite, "test_01", test_01);
    CU_add_test(*suite, "test_02", test_02);
    CU_add_test(*suite, "test_03", test_03);
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main(void)
{
    unsigned rv = 1;
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
