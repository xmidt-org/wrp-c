# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

project(wrp-c VERSION 1.1.1)

math(EXPR wrp-c_BITWISE_VERSION "${wrp-c_VERSION_MAJOR} << 16 | ${wrp-c_VERSION_MINOR} << 8 | ${wrp-c_VERSION_PATCH}" OUTPUT_FORMAT HEXADECIMAL)

set(CIMPLOG_PATH "" CACHE PATH "An alternate path to examine for the cimplog package")
set(MSGPACK_PATH "" CACHE PATH "An alternate path to examine for the msgpack package")
set(TROWER_PATH "" CACHE PATH "An alternate path to examine for the trower-base64 package")

add_definitions(-std=c99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

include(CTest)
include(ExternalProject)

add_library(wrp-c SHARED "")


set(LOCAL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_install)
set(LOCAL_PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/_prefix)
set(LOCAL_INCLUDE_DIR ${LOCAL_INSTALL_DIR}/include)
set(LOCAL_LIBRARY_DIR ${LOCAL_INSTALL_DIR}/lib${LIB_SUFFIX})

################################################################################
# Find or acquire software dependencies
################################################################################

include(FindMsgpack)
include(FindTrowerBase64)
include(FindCimplog)

################################################################################
# Process the subdirectories now
################################################################################

add_subdirectory(src)

if (BUILD_TESTING)
    include(FindCunit)
	add_subdirectory(tests)
endif (BUILD_TESTING)

add_custom_target(license
                  COMMAND reuse lint
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
