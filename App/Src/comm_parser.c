#include "comm_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

static bool parse_int(const char *s, long *out)
{
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0') { return false; }
    *out = v;
    return true;
}

void Comm_ParseLine(char *line, CommCmd_t *cmd)
{
    char *sp = NULL;
    char *t0, *t1, *t2, *t3;
    long  v;

    cmd->type  = CMD_ERR_INVALID_COMMAND;
    cmd->value = 0;

    for (char *p = line; *p != '\0'; ++p) {
        *p = (char)toupper((unsigned char)*p);
    }

    t0 = strtok_r(line, " ", &sp);
    t1 = strtok_r(NULL, " ", &sp);
    t2 = strtok_r(NULL, " ", &sp);
    t3 = strtok_r(NULL, " ", &sp);

    if (t0 == NULL) { cmd->type = CMD_NONE; return; }

    if (strcmp(t0, "GET") == 0 && t1 != NULL && strcmp(t1, "DATA") == 0 && t2 == NULL) {
        cmd->type = CMD_GET_DATA;
        return;
    }

    if (strcmp(t0, "SET") == 0 && t1 != NULL && t2 != NULL && t3 == NULL) {
        if (strcmp(t1, "DIST") == 0) {
            if (parse_int(t2, &v) && v >= COMM_DIST_MIN && v <= COMM_DIST_MAX) {
                cmd->type = CMD_SET_DIST; cmd->value = v;
            } else {
                cmd->type = CMD_ERR_INVALID_VALUE;
            }
        } else if (strcmp(t1, "SOURCE") == 0) {
            if      (strcmp(t2, "POT") == 0) { cmd->type = CMD_SET_SOURCE; cmd->value = 0; }
            else if (strcmp(t2, "PC")  == 0) { cmd->type = CMD_SET_SOURCE; cmd->value = 1; }
            else                             { cmd->type = CMD_ERR_INVALID_VALUE; }
        }
    }
}