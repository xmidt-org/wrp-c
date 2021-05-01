# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-error=missing-field-initializers -W -g -fprofile-arcs -ftest-coverage -O0")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W  -g -fprofile-arcs -ftest-coverage -O0")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -O0")
if(NOT DISABLE_VALGRIND)
set (MEMORY_CHECK valgrind --leak-check=full --show-reachable=yes --error-exitcode=1 -v)
endif ()

add_test(NAME Simple COMMAND ${MEMORY_CHECK} ./simple)
add_executable(simple simple.c ../src/wrp-c.c)
set_property(TARGET simple PROPERTY C_STANDARD 99)

target_link_libraries (simple -lmsgpackc -ltrower-base64 -lcimplog)
target_link_libraries (simple cunit)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
target_link_libraries (simple gcov)
target_link_libraries (simple rt)
endif()

add_custom_target(coverage
                  COMMAND lcov -q --capture --directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/simple.dir/__/src --output-file coverage.info
                  COMMAND genhtml coverage.info
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
