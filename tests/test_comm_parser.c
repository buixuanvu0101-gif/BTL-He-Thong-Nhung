#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "comm_parser.h"

static CommCmd_t run(const char *s)
{
    char buf[40];
    CommCmd_t c;
    strncpy(buf, s, sizeof(buf)); buf[sizeof(buf) - 1] = '\0';
    Comm_ParseLine(buf, &c);
    return c;
}

int main(void)
{
    assert(run("SET DIST 25").type == CMD_SET_DIST && run("SET DIST 25").value == 25);
    assert(run("set dist 25").type == CMD_SET_DIST);          /* chữ thường */
    assert(run("SET DIST 1").type == CMD_ERR_INVALID_VALUE);
    assert(run("SET DIST 401").type == CMD_ERR_INVALID_VALUE);
    assert(run("SET DIST abc").type == CMD_ERR_INVALID_VALUE);
    assert(run("SET DIST 25 9").type == CMD_ERR_INVALID_COMMAND);
    assert(run("SET DIST").type == CMD_ERR_INVALID_COMMAND);
    assert(run("SET SOURCE PC").value == 1);
    assert(run("SET SOURCE XYZ").type == CMD_ERR_INVALID_VALUE);
    assert(run("GET DATA").type == CMD_GET_DATA);
    assert(run("HELLO").type == CMD_ERR_INVALID_COMMAND);
    assert(run("").type == CMD_NONE);
    puts("comm_parser: OK");
    return 0;
}