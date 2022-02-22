/* SPDX-FileCopyrightText: 2010-2022 Comcast Cable Communications Management, LLC */
/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __WRP_C_H__
#define __WRP_C_H__

#include <stdint.h>

/*----------------------------------------------------------------------------*/
/*                                Return Codes                                */
/*----------------------------------------------------------------------------*/

/* All possible error codes from all the wrp-c functions. Future versions
 * may return other values.
 *
 * Always add new return codes last.  Do not remove any.  The return codes
 * must remain the same.
 */
typedef enum {
    WRPE_OK = 0,
    WRPE_OUT_OF_MEMORY,      /*  1 */
    WRPE_NOT_MSGPACK_FORMAT, /*  2 */
    WRPE_MSG_TOO_BIG,        /*  3 */
    WRPE_OTHER_ERROR,        /*  4 */
    WRPE_NOT_A_WRP_MSG,      /*  5 */
    WRPE_INVALID_ARGS,       /*  6 */
    WRPE_NOT_FROM_WRPC,      /*  7 */
    WRPE_NO_SCHEME,          /*  8 */
    WRPE_NO_AUTHORITY,       /*  9 */

    WRPE_LAST /* never use! */
} WRPcode;

/*----------------------------------------------------------------------------*/
/*                             Common Structures                              */
/*----------------------------------------------------------------------------*/
struct wrp_string {
    size_t len;
    const char *s;
};

struct wrp_string_list {
    size_t count;
    struct wrp_string *list;
};

struct wrp_int {
    int *num;
    int __internal_only; /* Don't directly use this. */
};

struct wrp_blob {
    size_t len;
    const uint8_t *data;
};

struct wrp_nvp {
    struct wrp_string name;
    struct wrp_string value;
};

struct wrp_nvp_list {
    size_t count;
    struct wrp_nvp *list;
};


/*----------------------------------------------------------------------------*/
/*                             Locator Structure                              */
/*----------------------------------------------------------------------------*/

/*
 *   [scheme]:[authority]/[service]/[app]
 *
 *   mac:112233445566/parodus
 *   mac:112233445566/parodus/extra_parodus/stuff/that-is+ignored
 *   event:status/target/other_stuff/is/ignored
 *   dns:foo.example.com:8443/source/stuff
 */

typedef struct {
    struct wrp_string scheme;
    struct wrp_string authority;
    struct wrp_string service;
    struct wrp_string app;
} wrp_locator_t;


/*----------------------------------------------------------------------------*/
/*                              WRP Structures                                */
/*----------------------------------------------------------------------------*/

// clang-format off
enum wrp_msg_type {
    WRP_MSG_TYPE__AUTH      = 2,
    WRP_MSG_TYPE__REQ       = 3,
    WRP_MSG_TYPE__EVENT     = 4,
    WRP_MSG_TYPE__CREATE    = 5,
    WRP_MSG_TYPE__RETRIEVE  = 6,
    WRP_MSG_TYPE__UPDATE    = 7,
    WRP_MSG_TYPE__DELETE    = 8,
    WRP_MSG_TYPE__SVC_REG   = 9,
    WRP_MSG_TYPE__SVC_ALIVE = 10,
};


struct wrp_auth_msg {
    struct wrp_int status;                  /* Required */
};

struct wrp_req_msg {
    struct wrp_string       dest;           /* Required */
    struct wrp_string       source;         /* Required */
    struct wrp_blob         payload;        /* Required */
    struct wrp_string       trans_id;       /* Required */

    struct wrp_string       accept;         /* Optional */
    struct wrp_string       content_type;   /* Optional */
    struct wrp_string_list  headers;        /* Optional */
    struct wrp_nvp_list     metadata;       /* Optional */
    struct wrp_string       msg_id;         /* Optional */
    struct wrp_string_list  partner_ids;    /* Optional */
    struct wrp_int          rdr;            /* Optional */
    struct wrp_string       session_id;     /* Optional */
    struct wrp_int          status;         /* Optional */
};

struct wrp_event_msg {
    struct wrp_string       dest;           /* Required */
    struct wrp_string       source;         /* Required */

    struct wrp_string       content_type;   /* Optional */
    struct wrp_string_list  headers;        /* Optional */
    struct wrp_nvp_list     metadata;       /* Optional */
    struct wrp_string       msg_id;         /* Optional */
    struct wrp_string_list  partner_ids;    /* Optional */
    struct wrp_blob         payload;        /* Optional */
    struct wrp_int          rdr;            /* Optional */
    struct wrp_string       session_id;     /* Optional */
    struct wrp_string       trans_id;       /* Optional */
};

struct wrp_crud_msg {
    struct wrp_string       dest;           /* Required */
    struct wrp_string       source;         /* Required */
    struct wrp_string       trans_id;       /* Required */

    struct wrp_string       accept;         /* Optional */
    struct wrp_string       content_type;   /* Optional */
    struct wrp_string_list  headers;        /* Optional */
    struct wrp_nvp_list     metadata;       /* Optional */
    struct wrp_string       msg_id;         /* Optional */
    struct wrp_string_list  partner_ids;    /* Optional */
    struct wrp_string       path;           /* Optional */
    struct wrp_blob         payload;        /* Optional */
    struct wrp_int          rdr;            /* Optional */
    struct wrp_string       session_id;     /* Optional */
    struct wrp_int          status;         /* Optional */
};

struct wrp_svc_reg_msg {
    struct wrp_string service_name;         /* Required */
    struct wrp_string url;                  /* Required */
};

typedef struct {
    enum wrp_msg_type msg_type;
    struct wrp_string *source;  /* Points to the source if present. */
    struct wrp_string *dest;    /* Points to the dest if present. */

    union {
        struct wrp_auth_msg     auth;
        struct wrp_crud_msg     crud;
        struct wrp_event_msg    event;
        struct wrp_svc_reg_msg  reg;
        struct wrp_req_msg      req;
    } u;

    struct wrp_blob original;   /* The original message for convenience. This is
                                 * not free()d by wrp_destroy(), but is a
                                 * convenient place to pass along the original
                                 * data that is needed by this structure. */

    void *__internal_only;      /* For internal use, don't modify this. */
} wrp_msg_t;
// clang-format on

/*----------------------------------------------------------------------------*/
/*                              WRP Functions                                 */
/*----------------------------------------------------------------------------*/

/**
 *  Converts a buffer with a msgpack encoded wrp into the c structure.
 *
 *  @note The resulting message structure references the original data.
 *        The user should call wrp_destroy() on the returned msg before
 *        releaseing the original data object.
 *
 *  @param src  the buffer with the msgpack data
 *  @param len  the length of the src buffer
 *  @param dest the resulting object (must be released)
 *
 *  @retval WRPE_OK
 *  @retval WRPE_INVALID_ARGS
 *  @retval WRPE_NOT_MSGPACK_FORMAT
 *  @retval WRPE_NOT_A_WRP_MSG
 *  @retval WRPE_MSG_TOO_BIG
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_OTHER_ERROR
 */
WRPcode wrp_from_msgpack(const void *src, size_t len, wrp_msg_t **dest);


/**
 *  Converts a wrp structure to a message pack encoded form, either in a user
 *  specified buffer or one allocated by the function.
 *
 *  @param src  the message to encode
 *  @param dest If *dest != NULL then encode into the specified buffer.
 *              If *dest == NULL then the function will allocate a buffer and
 *              return a pointer to it here.
 *  @param len  The dest buffer length if provided, and the number of valid
 *              encoded byte in dest.
 *
 *  @retval WRPE_OK
 *  @retval WRPE_INVALID_ARGS
 *  @retval WRPE_NOT_A_WRP_MSG
 *  @retval WRPE_MSG_TOO_BIG
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_OTHER_ERROR
 */
WRPcode wrp_to_msgpack(const wrp_msg_t *src, uint8_t **dest, size_t *len);


/**
 *  Cleans up the allocations from the msg.
 *
 *  @retval WRPE_OK
 *  @retval WRPE_NOT_FROM_WRPC
 */
WRPcode wrp_destroy(wrp_msg_t *msg);


/**
 *  Prints a wrp_msg_t structure into an array of bytes that must be freed.
 *
 *  @param msg the message to convert
 *  @param dst the resulting buffer
 *  @param len the resultint length
 *
 *  @retval WRPE_OK
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_INVALID_ARGS
 */
WRPcode wrp_to_string(const wrp_msg_t *msg, char **dst, size_t *len);


/*----------------------------------------------------------------------------*/
/*                             Locator Functions                              */
/*----------------------------------------------------------------------------*/

/**
 *  Gets a specific part of the specified locator (source or dest fields
 *  typically).  The response parts are references to the input string.
 *
 *  @param loc  the locator to examine
 *  @param len  the length of the loc field
 *  @param out  the locator struct to populate based on the loc string
 *
 *  @retval WRP_OK
 *  @retval WRPE_INVALID_ARGS
 *  @retval WRPE_NO_SCHEME
 *  @retval WRPE_NO_AUTHORITY
 */
WRPcode wrp_loc_split(const char *loc, size_t len, wrp_locator_t *out);


/**
 *  Prints a wrp_locator_t structure into an array of bytes that must be freed.
 *
 *  @param loc the locator to convert
 *  @param dst the resulting buffer
 *  @param len the resultint length
 *
 *  @retval WRP_OK
 *  @retval WRPE_OUT_OF_MEMORY
 *  @retval WRPE_INVALID_ARGS
 */
WRPcode wrp_loc_to_string(const wrp_locator_t *loc, char **dst, size_t *len);
#endif
