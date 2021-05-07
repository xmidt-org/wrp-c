# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

# find_cimplog( [PATH "path"] [VERSION "1.1.2"] [GIT_TAG "v1.2.3"] )
#   PATH    - An alternate path to examine for the cimplog package
#   VERSION - The expected version of the cimplog package
#   GIT_TAG - The specific git tag to checkout if it comes to that

# Search Order:
#   1. User specified path
#   2. Existing installed path
#   3. Fetch a working copy & build it

function(find_cimplog)

    cmake_parse_arguments(CIMPLOG "" "PATH;VERSION;GIT_TAG" "" ${ARGN})

    include(LocalInstallPaths)

    #message(STATUS "CIMPLOG_PATH    = \"${CIMPLOG_PATH}\"")
    #message(STATUS "CIMPLOG_VERSION = \"${CIMPLOG_VERSION}\"")
    #message(STATUS "CIMPLOG_GIT_TAG = \"${CIMPLOG_GIT_TAG}\"")

    if (NOT DEFINED CIMPLOG_VERSION)
        set(CIMPLOG_ENFORCE_VERSION "cimplog")
        if (NOT DEFINED CIMPLOG_GIT_TAG)
            set(CIMPLOG_GIT_TAG "")
        endif()
    else ()
        set(CIMPLOG_ENFORCE_VERSION "cimplog>=${CIMPLOG_VERSION}")
        if (NOT DEFINED CIMPLOG_GIT_TAG)
            set(CIMPLOG_GIT_TAG "v${CIMPLOG_VERSION}")
        endif()
    endif()

    find_path(CIMPLOG_INCLUDE_DIR
        NAMES "cimplog/cimplog.h"
        PATHS "${CMAKE_CURRENT_BINARY_DIR}/${CIMPLOG_PATH}"
        PATH_SUFFIXES "include"
        NO_DEFAULT_PATH)

    find_library(CIMPLOG_LIBRARY_DIR
                 NAMES "libcimplog.so"
                 PATHS "${CMAKE_CURRENT_BINARY_DIR}/${CIMPLOG_PATH}"
                 PATH_SUFFIXES "lib" "lib64"
                 NO_DEFAULT_PATH)

    if (NOT (CIMPLOG_INCLUDE_DIR MATCHES "-NOTFOUND" OR CIMPLOG_LIBRARY_DIR MATCHES "-NOTFOUND"))
        message(STATUS "Found user specified cimplog (at: \"${CIMPLOG_PATH}\")")
        include_directories(SYSTEM ${CIMPLOG_INCLUDE_DIR})
        set(CIMPLOG_LIBRARIES ${CIMPLOG_LIBRARY_DIR} PARENT_SCOPE)
    else()
        include(FindPkgConfig)

        pkg_check_modules(CIMPLOG QUIET ${CIMPLOG_ENFORCE_VERSION})
        if (CIMPLOG_FOUND EQUAL 1)
            message(STATUS "Using system provided cimplog (found version \"${CIMPLOG_VERSION}\")")
            include_directories(SYSTEM ${CIMPLOG_INCLUDE_DIRS})
        else()
            include(ExternalProject)
            
            message(STATUS "Fetching upstream cimplog")
            ExternalProject_Add(cimplog
                PREFIX ${LOCAL_PREFIX_DIR}/cimplog
                GIT_REPOSITORY https://github.com/xmidt-org/cimplog.git
                GIT_TAG ${CIMPLOG_GIT_TAG}
                CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_DIR} -DBUILD_TESTING=OFF)
            add_library(libcimplog STATIC IMPORTED)
            add_dependencies(libcimplog cimplog)
            add_dependencies(${CMAKE_PROJECT_NAME} cimplog)
            include_directories(SYSTEM ${LOCAL_INCLUDE_DIR})
            set(CIMPLOG_LIBRARIES "${LOCAL_LIBRARY_DIR}/libcimplog.so" PARENT_SCOPE)
        endif()
    endif()
endfunction()
