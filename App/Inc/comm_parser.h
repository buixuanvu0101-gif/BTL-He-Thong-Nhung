#ifndef COMM_PARSER_H
#define COMM_PARSER_H

#include <stdint.h>
#include "app_cfg.h"

typedef enum {
    CMD_NONE = 0,               /* dong rong */
    CMD_GET_DATA,
    CMD_SET_DIST,               /* value = cm */
    CMD_SET_SOURCE,             /* value = 0 (ENC) hoac 1 (PC) */
    CMD_ERR_INVALID_COMMAND,
    CMD_ERR_INVALID_VALUE
} CommCmdType_t;

typedef struct {
    CommCmdType_t type;
    long          value;
} CommCmd_t;

/* Sua truc tiep chuoi line (chuyen chu hoa, cat tu) */
void Comm_ParseLine(char *line, CommCmd_t *cmd);

#endif
