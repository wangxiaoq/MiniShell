#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "history-cmd.h"

#define MAX_HISTORY_ITEM 1000

/*
 * full path: ~/.minish_history
 */
#define HISTORY_FILE "/.minish_history"
static char history_file_full_path[NAMELEN];
/*
 * FIFO queue.
 * cmd_fifo_head -> the oldest cmd item
 * cmd_fifo_tail -> the newest cmd item
 * cmd_fifo_cur  -> current cmd item
 *
 *             cur
 * tail->item->item->.........->head
 */
static cmd_item_t *cmd_fifo_head = NULL;
static cmd_item_t *cmd_fifo_tail = NULL;
static cmd_item_t *cmd_fifo_cur = NULL;

static int history_item_num = 0;

static inline int get_history_file_full_path(void)
{
    char *home = get_user_home();
    if (home == NULL) {
        return -1;
    }

    memset(history_file_full_path, 0, NAMELEN);
    strcpy(history_file_full_path, home);
    strcat(history_file_full_path, HISTORY_FILE);

    return 0;
}

static int load_history_cmd_file(void)
{
    char cmd_str[CMDLINE_MAXLENGTH] = {0};
    int ret = 0;
    FILE *fp = fopen(history_file_full_path, "r");
    if (fp == NULL) {
        return 0;
    }

    while (fgets(cmd_str, CMDLINE_MAXLENGTH, fp) != NULL) {
        /* skip the newline at end */
        if (cmd_str[strlen(cmd_str) - 1] == '\n') {
            cmd_str[strlen(cmd_str) - 1] = '\0';
        }

        ret = add_cmd_item(cmd_str);
        if (ret) {
            return -1;
        }
    }

    fclose(fp);

    return 0;
}

static int save_history_cmd_file(void)
{
    cmd_item_t *citem = cmd_fifo_head;
    cmd_item_t *tmp = citem;
    /* truncate file length to 0 */
    FILE *fp = fopen(history_file_full_path, "w");
    if (fp == NULL) {
        return -1;
    }

    while (citem) {
        fprintf(fp, "%s\n", citem->cmd);
        tmp = citem;
        citem = citem->prev;
        free(tmp);
    }

    fclose(fp);

    return 0;
}

static void del_oldest_cmd_item(void)
{
    cmd_item_t * citem = NULL;

    if (cmd_fifo_head) {
        citem = cmd_fifo_head;
        cmd_fifo_head = cmd_fifo_head->prev;
        free(citem);
        history_item_num--;
    }

    return ;
}

int add_cmd_item(char *cmd_str)
{
    cmd_item_t *cmd_item = NULL;
    /* skip the empty line */
    if (strlen(cmd_str) == 0) {
        return 0;
    }

    cmd_item = malloc(sizeof(*cmd_item) + strlen(cmd_str) + 1);
    if (cmd_item == NULL) {
        return -1;
    }
    cmd_item->prev = NULL;
    cmd_item->next = NULL;
    strcpy(cmd_item->cmd, cmd_str);

    /* insert at the fifo_queue tail */
    if (cmd_fifo_tail == NULL) {
        cmd_fifo_tail = cmd_item;
        cmd_fifo_head = cmd_item;
        cmd_fifo_cur = NULL;
        history_item_num++;
    } else {
        cmd_item->next = cmd_fifo_tail;
        cmd_fifo_tail->prev = cmd_item;
        cmd_fifo_tail = cmd_item;
        cmd_fifo_cur = NULL;
        history_item_num++;
    }

    if (history_item_num > MAX_HISTORY_ITEM) {
        del_oldest_cmd_item();
    }

    return 0;
}

char *up_cmd_item(void)
{
    if (cmd_fifo_cur == NULL) {
        cmd_fifo_cur = cmd_fifo_tail;
        return cmd_fifo_cur->cmd;
    } else if (cmd_fifo_cur == cmd_fifo_head) {
        return cmd_fifo_cur->cmd;
    } else {
        cmd_fifo_cur = cmd_fifo_cur->next;
        return cmd_fifo_cur->cmd;
    }
}

char *down_cmd_item(void)
{
    if (cmd_fifo_cur == NULL) {
        return NULL;
    } else {
        cmd_fifo_cur = cmd_fifo_cur->prev;
        return (cmd_fifo_cur == NULL) ? NULL : cmd_fifo_cur->cmd;
    }
}


int init_history_cmd(void)
{
    int ret = get_history_file_full_path();
    if (ret < 0) {
        return -1;
    }
    ret = load_history_cmd_file();
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int destroy_history_cmd(void)
{
    int ret = save_history_cmd_file();
    if (ret < 0) {
        return -1;
    }

    return 0;
}

void dump_history_cmd(void)
{
    cmd_item_t *citem = cmd_fifo_head;

    while (citem) {
        fprintf(stdout, "%s\n", citem->cmd);
        citem = citem->prev;
    }

    fprintf(stdout, "%d items.\n", history_item_num);

    return ;
}
