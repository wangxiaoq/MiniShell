#ifndef __UTIL_H
#define __UTIL_H

#define NAMELEN 512
#define CMDLINE_MAXLENGTH 4096

char *get_current_user(void);
char *get_user_home(void);
void print_prompt(void);
int is_dir(char *file);

#endif
