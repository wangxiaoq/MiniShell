#ifndef __HISTORY_CMD_H
#define __HISTORY_CMD_H

typedef struct cmd_item {
    struct cmd_item *prev;
    struct cmd_item *next;
    char cmd[];
} cmd_item_t;

int init_history_cmd(void);
int destroy_history_cmd(void);
int add_cmd_item(char *cmd_str);
char *up_cmd_item(void);
char *down_cmd_item(void);
void dump_history_cmd(void);

#endif
