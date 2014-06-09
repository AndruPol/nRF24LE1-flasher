/*
 * cmds.h
 */

#ifndef CMDS_H_
#define CMDS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "ch.h"
#include "hal.h"

#include "shell.h"

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_xmread(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_xmsend(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_read(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"xmread", cmd_xmread},
  {"xmsend", cmd_xmsend},
  {"dump", cmd_dump},
  {"test", cmd_test},
  {"write", cmd_write},
  {"read", cmd_read},
  {NULL, NULL}
};

#ifdef __cplusplus
}
#endif
#endif /* CMDS_H_ */

