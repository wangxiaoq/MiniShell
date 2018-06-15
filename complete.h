#ifndef __COMPLETE_H

#define __COMPLETE_H

int is_sys_executable_cmd(char *buf);
int complete_executable_cmd(char *buf);
int complete_cmd_with_path(char *buf);

#endif
