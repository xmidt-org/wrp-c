# SPDX-FileCopyrightText: 2016-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

target_sources(wrp-c
    PRIVATE
        b64.c
        msgpack.c
        string.c
        utils.c
        wrp-c.c
)

configure_file(wrp-c_ver.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/wrp-c_ver.h)

install (FILES
         ${CMAKE_CURRENT_BINARY_DIR}/wrp-c_ver.h
         ${CMAKE_CURRENT_SOURCE_DIR}/wrp-c.h
         DESTINATION include/wrp-c)

set_target_properties(wrp-c PROPERTIES
        OUTPUT_NAME wrp-c)

install(TARGETS wrp-c
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
       )

