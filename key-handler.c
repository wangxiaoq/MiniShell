#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include "history-cmd.h"
#include "util.h"

/* handle up arrow key */
static char *handle_up_key(char *buf, int buf_max_len)
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

    return cmd;
}

/* handle down arrow key */
char *handle_down_key(char *buf, char *cur_cmd, int buf_max_len)
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

    return cmd;
}

/* handle left arrow key */
void handle_left_key(char *buf, int buf_max_len)
{
    if (strlen(buf)) {
        printf("\033[1D");
        buf[strlen(buf) -1] = 0;
    }
    fflush(stdout);
}

/* handle delete key */
void handle_delete_key(char *buf, int buf_max_len)
{
    int len = 0;
    int i = 0;

    len = strlen(buf);
    if (len == 0) {
        return ;
    }
    buf[len-1] = '\0';
    len--;
    print_prompt();
    for (i = 0; i < len + 1; i++) {
        printf(" ");
    }
    print_prompt();
    fprintf(stdout, "%s", buf);

    return ;
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
                cmd = handle_up_key(buf, buf_max_len);
            } else if (ch == 66) { /* down key */
                cmd = handle_down_key(buf, cur_cmd, buf_max_len);
            } else if (ch == 68) {
                handle_left_key(buf, buf_max_len);
            //    printf("\033[1D");
            //    fflush(stdout);
            }
            break;

        case 127: /* delete key */
            handle_delete_key(buf, buf_max_len);
            break;

        case 10: /* enter key */
            flag = 0;
            ret = strlen(buf);
            break;

        case 4: /* CTRL+D */
            flag = 0;
            ret = -1;
            break;

        case -1: /* CTRL+C && CTRL+\ */
            flag = 0;
            ret = 0;
            memset(buf, 0, buf_max_len);
            break;
        case 9: /* tab key */
            break;

        default:
            len = strlen(buf);
            fprintf(stdout, "%c", ch);
          //  fprintf(stdout, "%d", (int8_t)ch);
            buf[len] = ch;
            len++;
            strcpy(cur_cmd, buf);
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
