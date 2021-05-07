# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

# Search Order:
#   1. User specified path
#   2. Existing installed path
#   3. Fetch a working copy & build it

set(MSGPACK_PATH "" CACHE PATH "An alternate path to examine for the msgpack package")

find_path(MSGPACK_INCLUDE_DIR
          NAMES "msgpack.h"
          HINTS "${MSGPACK_PATH}"
          PATH_SUFFIXES "include"
          NO_DEFAULT_PATH)

find_library(MSGPACK_LIBRARY_DIR
             NAMES "libmsgpackc.so"
             HINTS "${MSGPACK_PATH}"
             PATH_SUFFIXES "lib" "lib64"
             NO_DEFAULT_PATH)

if (NOT (MSGPACK_INCLUDE_DIR MATCHES "-NOTFOUND" OR MSGPACK_LIBRARY_DIR MATCHES "-NOTFOUND"))
    message(STATUS "Found user specified msgpack (at: \"${MSGPACK_PATH}\")")
    include_directories(SYSTEM ${MSGPACK_INCLUDE_DIR})
    set(MSGPACK_LIBRARIES ${MSGPACK_LIBRARY_DIR})
else()
    include(FindPkgConfig)

    pkg_check_modules(MSGPACK QUIET msgpack>=2.1.5)
    if (MSGPACK_FOUND EQUAL 1)
        message(STATUS "Using system provided msgpack (found version \"${MSGPACK_VERSION}\")")
        include_directories(SYSTEM ${MSGPACK_INCLUDE_DIRS})
    else()
        message(STATUS "Fetching upstream msgpack")
        ExternalProject_Add(msgpack
            PREFIX ${LOCAL_PREFIX_DIR}/msgpack
            GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
            GIT_TAG "7a98138f27f27290e680bf8fbf1f8d1b089bf138"
            CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_DIR}
            -DMSGPACK_ENABLE_CXX=OFF
            -DMSGPACK_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF)
        add_library(libmsgpack STATIC IMPORTED)
        add_dependencies(libmsgpack msgpack)
        add_dependencies(wrp-c msgpack)
        include_directories(SYSTEM ${LOCAL_INCLUDE_DIR})
        set(MSGPACK_LIBRARIES "${LOCAL_LIBRARY_DIR}/libmsgpackc.so")
    endif()
endif()
