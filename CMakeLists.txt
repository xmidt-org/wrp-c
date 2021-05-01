# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 2.8.7)

project(wrp-c)
include(ExternalProject)
include(CTest)

add_definitions(-std=c99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g -Werror -Wall")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
endif()

set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_install)
set(PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/_prefix)
set(INCLUDE_DIR ${INSTALL_DIR}/include)
set(LIBRARY_DIR ${INSTALL_DIR}/lib${LIB_SUFFIX})

include_directories(${INCLUDE_DIR}
                    ${INCLUDE_DIR}/msgpack
                    ${INCLUDE_DIR}/trower-base64
                    ${INCLUDE_DIR}/cimplog)

include_directories (SYSTEM /usr/include)
link_directories ( ${LIBRARY_DIR})
                   
if (MAIN_PROJ_BUILD) 
include_directories(
                    ${MAIN_PROJ_INCLUDE_PATH}
                    ${MAIN_PROJ_INCLUDE_PATH}/msgpack
                    ${MAIN_PROJ_INCLUDE_PATH}/trower-base64
                    ${MAIN_PROJ_INCLUDE_PATH}/cimplog)
link_directories ( ${LIBRARY_DIR} ${MAIN_PROJ_LIB_PATH})                    
endif ()                    


enable_testing()

if (NOT BUILD_YOCTO)

# base64 external dependency
if (NOT MAIN_PROJ_BUILD)

ExternalProject_Add(trower-base64
    PREFIX ${PREFIX_DIR}/trower-base64
    GIT_REPOSITORY https://github.com/Comcast/trower-base64.git
    GIT_TAG "v1.1.2"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DBUILD_TESTING=OFF)
add_library(libtrower-base64 STATIC IMPORTED)
add_dependencies(libtrower-base64 trower-base64)

# msgpack-c external dependency
ExternalProject_Add(msgpack
    PREFIX ${PREFIX_DIR}/msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG "7a98138f27f27290e680bf8fbf1f8d1b089bf138"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    -DMSGPACK_ENABLE_CXX=OFF
    -DMSGPACK_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF)
add_library(libmsgpack STATIC IMPORTED)
add_dependencies(libmsgpack msgpack)

# cimplog external dependency
ExternalProject_Add(cimplog
    PREFIX ${PREFIX_DIR}/cimplog
    GIT_REPOSITORY https://github.com/Comcast/cimplog.git
    GIT_TAG "8a5fb3c2f182241d17f5342bea5b7688c28cd1fd"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                  -DBUILD_TESTING=OFF
)
add_library(libcimplog STATIC IMPORTED)
add_dependencies(libcimplog cimplog)

endif ()
endif ()

add_subdirectory(src)

if (BUILD_TESTING)
	add_subdirectory(tests)
endif (BUILD_TESTING)
