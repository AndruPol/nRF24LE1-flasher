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
void cmd_xmread(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_xmsend(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_read(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_nvwrite(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_nvread(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_ipread(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_ipwrite(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_erase_all(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"test", cmd_test},
  {"dump", cmd_dump},
  {"xmread", cmd_xmread},
  {"xmsend", cmd_xmsend},
  {"write", cmd_write},
  {"read", cmd_read},
  {"nvwrite", cmd_nvwrite},
  {"nvread", cmd_nvread},
  {"ipread", cmd_ipread},
  {"ipwrite", cmd_ipwrite},
  {"erase_all", cmd_erase_all},
  {NULL, NULL}
};

#ifdef __cplusplus
}
#endif
#endif /* CMDS_H_ */

