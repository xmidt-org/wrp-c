# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

project(wrp-c VERSION 1.1.1)

include(BitwiseVersion)
include(CTest)
include(FindCimplog)
include(FindMsgpack)
include(FindTrowerBase64)
include(LicenseLinterTarget)

add_definitions(-std=c99)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

add_library(wrp-c SHARED "")


################################################################################
# Find or acquire software dependencies
################################################################################

find_msgpack(      PATH ${MSGPACK_PATH} VERSION "2.1.5" GIT_TAG "7a98138f27f27290e680bf8fbf1f8d1b089bf138")
find_trower_base64(PATH ${TROWER_PATH}                  GIT_TAG "")
find_cimplog(      PATH ${CIMPLOG_PATH}                 GIT_TAG "8a5fb3c2f182241d17f5342bea5b7688c28cd1fd")

################################################################################
# Process the subdirectories now
################################################################################

add_subdirectory(src)

if (BUILD_TESTING)
    include(FindCunit)
	add_subdirectory(tests)
endif (BUILD_TESTING)
