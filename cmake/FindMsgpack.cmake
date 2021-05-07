# SPDX-FileCopyrightText: 2010-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

# find_msgpack( [PATH "path"] [VERSION "1.1.2"] [GIT_TAG "v1.2.3"] )
#   PATH    - An alternate path to examine for the msgpack package
#   VERSION - The expected version of the msgpack package
#   GIT_TAG - The specific git tag to checkout if it comes to that

# Search Order:
#   1. User specified path
#   2. Existing installed path
#   3. Fetch a working copy & build it

function(find_msgpack)

    cmake_parse_arguments(MSGPACK "" "PATH;VERSION;GIT_TAG" "" ${ARGN})

    include(LocalInstallPaths)

    #message(STATUS "MSGPACK_PATH    = \"${MSGPACK_PATH}\"")
    #message(STATUS "MSGPACK_VERSION = \"${MSGPACK_VERSION}\"")
    #message(STATUS "MSGPACK_GIT_TAG = \"${MSGPACK_GIT_TAG}\"")

    if (NOT DEFINED MSGPACK_VERSION)
        set(MSGPACK_ENFORCE_VERSION "msgpack")
        if (NOT DEFINED MSGPACK_GIT_TAG)
            set(MSGPACK_GIT_TAG "")
        endif()
    else ()
        set(MSGPACK_ENFORCE_VERSION "msgpack>=${MSGPACK_VERSION}")
        if (NOT DEFINED MSGPACK_GIT_TAG)
            set(MSGPACK_GIT_TAG "v${MSGPACK_VERSION}")
        endif()
    endif()

    find_path(MSGPACK_INCLUDE_DIR
              NAMES "msgpack.h"
              PATHS "${CMAKE_CURRENT_BINARY_DIR}/${MSGPACK_PATH}"
              PATH_SUFFIXES "include"
              NO_DEFAULT_PATH)

    find_library(MSGPACK_LIBRARY_DIR
                 NAMES "libmsgpackc.so"
                 PATHS "${CMAKE_CURRENT_BINARY_DIR}/${MSGPACK_PATH}"
                 PATH_SUFFIXES "lib" "lib64"
                 NO_DEFAULT_PATH)

    if (NOT (MSGPACK_INCLUDE_DIR MATCHES "-NOTFOUND" OR MSGPACK_LIBRARY_DIR MATCHES "-NOTFOUND"))
        message(STATUS "Found user specified msgpack (at: \"${MSGPACK_PATH}\")")
        include_directories(SYSTEM ${MSGPACK_INCLUDE_DIR})
        set(MSGPACK_LIBRARIES ${MSGPACK_LIBRARY_DIR} PARENT_SCOPE)
    else()
        include(FindPkgConfig)

        pkg_check_modules(MSGPACK QUIET ${MSGPACK_ENFORCE_VERSION})
        if (MSGPACK_FOUND EQUAL 1)
            message(STATUS "Using system provided msgpack (found version \"${MSGPACK_VERSION}\")")
            include_directories(SYSTEM ${MSGPACK_INCLUDE_DIRS})
        else()
            include(ExternalProject)

            message(STATUS "Fetching upstream msgpack")
            ExternalProject_Add(msgpack
                PREFIX ${LOCAL_PREFIX_DIR}/msgpack
                GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
                GIT_TAG ${MSGPACK_GIT_TAG}
                CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_DIR}
                -DMSGPACK_ENABLE_CXX=OFF
                -DMSGPACK_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF)
            add_library(libmsgpack STATIC IMPORTED)
            add_dependencies(libmsgpack msgpack)
            add_dependencies(${CMAKE_PROJECT_NAME} msgpack)
            include_directories(SYSTEM ${LOCAL_INCLUDE_DIR})
            set(MSGPACK_LIBRARIES "${LOCAL_LIBRARY_DIR}/libmsgpackc.so" PARENT_SCOPE)
        endif()
    endif()
endfunction()
