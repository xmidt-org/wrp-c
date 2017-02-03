#   Copyright 2010-2016 Comcast Cable Communications Management, LLC
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

cmake_minimum_required(VERSION 2.8.7)

project(wrp-c)
include(ExternalProject)
include(CTest)

add_definitions(-std=c99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g -Werror -Wall")

set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_install)
set(PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/_prefix)
set(INCLUDE_DIR ${INSTALL_DIR}/include)
set(LIBRARY_DIR ${INSTALL_DIR}/lib${LIB_SUFFIX})
include_directories(${INCLUDE_DIR}
                    ${INCLUDE_DIR}/msgpack
                    ${INCLUDE_DIR}/trower-base64
                    ${INCLUDE_DIR}/cimplog)
include_directories (SYSTEM /usr/include)
link_directories ( ${LIBRARY_DIR} )

enable_testing()

if (NOT BUILD_YOCTO)

# base64 external dependency
ExternalProject_Add(trower-base64
    PREFIX ${PREFIX_DIR}/trower-base64
    GIT_REPOSITORY https://github.com/Comcast/trower-base64.git
    GIT_TAG "master"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR})
add_library(libtrower-base64 STATIC IMPORTED)
add_dependencies(libtrower-base64 trower-base64)

# msgpack-c external dependency
ExternalProject_Add(msgpack
    PREFIX ${PREFIX_DIR}/msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    -DMSGPACK_ENABLE_CXX=OFF
    -DMSGPACK_BUILD_EXAMPLES=OFF)
add_library(libmsgpack STATIC IMPORTED)
add_dependencies(libmsgpack msgpack)

# cimplog external dependency
ExternalProject_Add(cimplog
    PREFIX ${PREFIX_DIR}/cimplog
    GIT_REPOSITORY https://github.com/Comcast/cimplog.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR})
add_library(libcimplog STATIC IMPORTED)
add_dependencies(libcimplog cimplog)

endif ()

add_subdirectory(src)

if (BUILD_TESTING)
	add_subdirectory(tests)
endif (BUILD_TESTING)
