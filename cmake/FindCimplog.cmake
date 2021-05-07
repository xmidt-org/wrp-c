# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

# Search Order:
#   1. User specified path
#   2. Existing installed path
#   3. Fetch a working copy & build it

set(CIMPLOG_PATH "" CACHE PATH "An alternate path to examine for the cimplog package")

find_path(CIMPLOG_INCLUDE_DIR
    NAMES "cimplog/cimplog.h"
    HINTS "${CIMPLOG_PATH}"
    PATH_SUFFIXES "include"
    NO_DEFAULT_PATH)

find_library(CIMPLOG_LIBRARY_DIR
             NAMES "libcimplog.so"
             HINTS "${CIMPLOG_PATH}"
             PATH_SUFFIXES "lib" "lib64"
             NO_DEFAULT_PATH)

if (NOT (CIMPLOG_INCLUDE_DIR MATCHES "-NOTFOUND" OR CIMPLOG_LIBRARY_DIR MATCHES "-NOTFOUND"))
    message(STATUS "Found user specified cimplog (at: \"${CIMPLOG_PATH}\")")
    include_directories(SYSTEM ${CIMPLOG_INCLUDE_DIR})
    set(CIMPLOG_LIBRARIES ${CIMPLOG_LIBRARY_DIR})
else()
    include(FindPkgConfig)

    pkg_check_modules(CIMPLOG QUIET cimplog)
    if (CIMPLOG_FOUND EQUAL 1)
        message(STATUS "Using system provided cimplog (found version \"${CIMPLOG_VERSION}\")")
        include_directories(SYSTEM ${CIMPLOG_INCLUDE_DIRS})
    else()
        message(STATUS "Fetching upstream cimplog")
        ExternalProject_Add(cimplog
            PREFIX ${LOCAL_PREFIX_DIR}/cimplog
            GIT_REPOSITORY https://github.com/xmidt-org/cimplog.git
            GIT_TAG "8a5fb3c2f182241d17f5342bea5b7688c28cd1fd"
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_DIR} -DBUILD_TESTING=OFF)
        add_library(libcimplog STATIC IMPORTED)
        add_dependencies(libcimplog cimplog)
        add_dependencies(wrp-c cimplog)
        include_directories(SYSTEM ${LOCAL_INCLUDE_DIR})
        set(CIMPLOG_LIBRARIES "${LOCAL_LIBRARY_DIR}/libcimplog.so")
    endif()
endif()
