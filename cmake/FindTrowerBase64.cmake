# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

# Search Order:
#   1. User specified path
#   2. Existing installed path
#   3. Fetch a working copy & build it

set(TROWER_PATH "" CACHE PATH "An alternate path to examine for the trower-base64 package")

find_path(TROWER_INCLUDE_DIR
          NAMES "trower-base64/base64.h"
          HINTS "${TROWER_PATH}"
          PATH_SUFFIXES "include"
          NO_DEFAULT_PATH)

find_library(TROWER_LIBRARY_DIR
             NAMES "libtrower-base64.so"
             HINTS "${TROWER_PATH}"
             PATH_SUFFIXES "lib" "lib64"
             NO_DEFAULT_PATH)

if (NOT (TROWER_INCLUDE_DIR MATCHES "-NOTFOUND" OR TROWER_LIBRARY_DIR MATCHES "-NOTFOUND"))
    message(STATUS "Found user specified trower-base64 (at: \"${TROWER_PATH}\")")
    include_directories(SYSTEM ${TROWER_INCLUDE_DIR})
    set(TROWER_LIBRARIES ${TROWER_LIBRARY_DIR})
else()
    include(FindPkgConfig)

    pkg_check_modules(TROWER_BASE64 QUIET trower-base64 trower-base64>=1.1.2)
    if (TROWER_BASE64_FOUND EQUAL 1)
        message(STATUS "Using system provided trower-base64 (found version \"${TROWER_BASE64_trower-base64_VERSION}\")")
        include_directories(SYSTEM ${TROWER_BASE64_INCLUDE_DIRS})
    else()
        message(STATUS "Fetching upstream trower-base64")
        ExternalProject_Add(trower-base64
            PREFIX ${LOCAL_PREFIX_DIR}/trower-base64
            GIT_REPOSITORY https://github.com/xmidt-org/trower-base64.git
            GIT_TAG "v1.1.2"
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_DIR} -DBUILD_TESTING=OFF)
        add_library(libtrower-base64 STATIC IMPORTED)
        add_dependencies(libtrower-base64 trower-base64)
        add_dependencies(wrp-c trower-base64)
        include_directories(SYSTEM ${LOCAL_INCLUDE_DIR})
    set(TROWER_LIBRARIES "${LOCAL_LIBRARY_DIR}/libtrower-base64.so")
    endif()
endif()
