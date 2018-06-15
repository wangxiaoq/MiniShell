#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include "history-cmd.h"
#include "util.h"
#include "complete.h"

static void flush_screen_after_input(char *buf, int *current_cursor)
{
    int len = strlen(buf);
    int i = 0;

    print_prompt();
    for (i = 0; i < len + 1; i++) {
        printf(" ");
    }
    print_prompt();
    fprintf(stdout, "%s", buf);
    /* adjust the cursor */
    for (i = 0; i < len - (*current_cursor); i++) {
        printf("\033[1D");
    }
}

/* handle up arrow key */
static char *handle_up_key(char *buf, int buf_max_len, int *current_cursor)
{
    int len = 0;
    char *cmd = NULL;
    int i = 0;

    len = strlen(buf);
    memset(buf, 0, buf_max_len);
    cmd = up_cmd_item();
    if (cmd != NULL) {
        strcpy(buf, cmd);
    }
    print_prompt();
    fflush(stdout);
    for (i = 0; i < len; i++) {
        printf(" ");
        fflush(stdout);
    }
    print_prompt();
    fprintf(stdout, "%s", buf);
    fflush(stdout);

    *current_cursor = strlen(buf);

    return cmd;
}

/* handle down arrow key */
static char *handle_down_key(char *buf, char *cur_cmd, int buf_max_len, int *current_cursor)
{
    int len = 0;
    char *cmd = NULL;
    int i = 0;

    len = strlen(buf);
    memset(buf, 0, buf_max_len);
    cmd = down_cmd_item();
    if (cmd != NULL) {
        strcpy(buf, cmd);
    } else {
        strcpy(buf, cur_cmd);
    }
    print_prompt();
    fflush(stdout);
    for (i = 0; i < len; i++) {
        printf(" ");
        fflush(stdout);
    }
    print_prompt();
    fflush(stdout);
    fprintf(stdout, "%s", buf);
    fflush(stdout);

    *current_cursor = strlen(buf);

    return cmd;
}

/* handle left arrow key */
static void handle_left_key(char *buf, int buf_max_len, int *current_cursor)
{
    if (*current_cursor > 0) {
        printf("\033[1D");
        (*current_cursor)--;
    }
    fflush(stdout);
}

/* handle right arrow key */
static void handle_right_key(char *buf, int buf_max_len, int *current_cursor)
{
    if (*current_cursor < strlen(buf)) {
        printf("\033[1C");
        (*current_cursor)++;
    }
    fflush(stdout);
}

/* handle delete key */
static void handle_delete_key(char *buf, int buf_max_len, int *current_cursor)
{
    int len = 0;
    int i = 0;

    len = strlen(buf);
    if (len == 0) {
        return ;
    }

    if (*current_cursor == 0) {
        return ;
    }

    if (*current_cursor != len) {
        for (i = (*current_cursor) - 1; i < len; i++) {
            buf[i] = buf[i+1];
        }
    } else {
        buf[len-1] = '\0';
    }
    (*current_cursor)--;
    len--;

    flush_screen_after_input(buf, current_cursor);

    return ;
}

/* tab key to complete */
static void handle_tab_key(char *buf, int *current_cursor)
{
    int num = 0;
    char *to_complete_buf = NULL;
    int is_arg = 0;

    if (strlen(buf) == 0) {
        return ;
    }

    to_complete_buf = strrchr(buf, ' ');
    if (to_complete_buf == NULL) {
        to_complete_buf = buf;
    } else {
        to_complete_buf++;
        is_arg = 1;
    }

    if (is_arg == 0 && is_sys_executable_cmd(to_complete_buf)) {
        num = complete_executable_cmd(to_complete_buf);
    } else {
        num = complete_cmd_with_path(to_complete_buf);
    }

    if (num == 1) {
        *current_cursor = strlen(buf);
    }
    flush_screen_after_input(buf, current_cursor);
}

/*
 * myread: handle all keys from keyboard input.
 *
 * return value: >= 0: number of character read
 *               < 0: end of file
 */
int myread(char *buf, int buf_max_len)
{
    struct termios old_opt, opt;
    char ch = 0;
    int len = 0, i = 0;
    char *cmd = NULL;
    int flag = 1;
    int ret = 0;
    char cur_cmd[buf_max_len];
    /* current position in buf */
    int current_cursor = 0;
    int tab_hit_times = 0;

    memset(cur_cmd, 0, buf_max_len);
    memset(buf, 0, buf_max_len);

    tcgetattr(0, &old_opt);
    opt = old_opt;
    opt.c_lflag &= ~ECHO;
    opt.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &opt);

    print_prompt();
    while (flag) {
        ch = getchar();
        switch (ch) {
        case 27:
            getchar();
            ch = getchar();
            /* up key */
            if (ch == 65) {
                cmd = handle_up_key(buf, buf_max_len, &current_cursor);
            } else if (ch == 66) { /* down key */
                cmd = handle_down_key(buf, cur_cmd, buf_max_len, &current_cursor);
            } else if (ch == 68) {
                handle_left_key(buf, buf_max_len, &current_cursor);
            } else if (ch == 67) {
                handle_right_key(buf, buf_max_len, &current_cursor);
            }
            tab_hit_times = 0;
            break;

        case 127: /* delete key */
            handle_delete_key(buf, buf_max_len, &current_cursor);
            tab_hit_times = 0;
            break;

        case 10: /* enter key */
            flag = 0;
            ret = strlen(buf);
            current_cursor = 0;
            tab_hit_times = 0;
            break;

        case 4: /* CTRL+D */
            flag = 0;
            ret = -1;
            tab_hit_times = 0;
            break;

        case -1: /* CTRL+C && CTRL+\ */
            flag = 0;
            ret = 0;
            memset(buf, 0, buf_max_len);
            tab_hit_times = 0;
            break;
        case 9: /* tab key */
            tab_hit_times++;
            if (tab_hit_times >= 2) {
                handle_tab_key(buf, &current_cursor);
                if (strlen(buf) == 0) {
                    tab_hit_times = 0;
                }
            }
            break;

        default:
            len = strlen(buf);
            fprintf(stdout, "%c", ch);
            if (current_cursor != len) {
                for (i = len; i >= current_cursor; i--) {
                    buf[i+1] = buf[i];
                }
                buf[current_cursor] = ch;
            } else {
                buf[len] = ch;
            }
            len++;
            current_cursor++;
            strcpy(cur_cmd, buf);
            flush_screen_after_input(buf, &current_cursor);
            tab_hit_times = 0;
            break;
        } /* end switch */
    } /* end while */

    tcsetattr(0, TCSANOW, &old_opt);
    fprintf(stdout, "\n");

    if (ret <= 0) {
        return ret;
    }

    if (strlen(buf)) {
        ret = add_cmd_item(buf);
        if (ret < 0) {
            return ret;
        }
    }

    return ret;
}
