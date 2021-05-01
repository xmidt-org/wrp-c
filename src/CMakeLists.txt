# SPDX-FileCopyrightText: 2016-2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

set(PROJ_WRPC wrp-c)

file(GLOB HEADERS wrp-c.h)
set(SOURCES wrp-c.c)

add_library(${PROJ_WRPC} STATIC ${HEADERS} ${SOURCES})
add_library(${PROJ_WRPC}.shared SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJ_WRPC}.shared PROPERTIES OUTPUT_NAME ${PROJ_WRPC})
set_property(TARGET ${PROJ_WRPC} PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJ_WRPC}.shared PROPERTY C_STANDARD 99)

install (TARGETS ${PROJ_WRPC} DESTINATION lib${LIB_SUFFIX})
install (TARGETS ${PROJ_WRPC}.shared DESTINATION lib${LIB_SUFFIX})
install (FILES wrp-c.h DESTINATION include/${PROJ_WRPC})
