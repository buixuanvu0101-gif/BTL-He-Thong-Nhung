#ifndef COMM_PARSER_H
#define COMM_PARSER_H

#include <stdint.h>

#define COMM_DIST_MIN  2L
#define COMM_DIST_MAX  400L

typedef enum {
    CMD_NONE = 0,               /* dòng rỗng */
    CMD_GET_DATA,
    CMD_SET_DIST,               /* value = cm */
    CMD_SET_SOURCE,             /* value = 0 (POT) hoặc 1 (PC) */
    CMD_ERR_INVALID_COMMAND,
    CMD_ERR_INVALID_VALUE
} CommCmdType_t;

typedef struct {
    CommCmdType_t type;
    long          value;
} CommCmd_t;

/* Sửa trực tiếp chuỗi line (chuyển chữ hoa, cắt từ) */
void Comm_ParseLine(char *line, CommCmd_t *cmd);

#endif