#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"
#include "key-handler.h"
#include "history-cmd.h"
#include "signal-handler.h"

#define MAX_ARGS 100

static int cmd_return_value = 0;

static void split_args(char *arg, char *parg[])
{
    int index = 1;
    char *p = NULL;

    if ((arg == NULL) || (*arg == 0)) {
        parg[index] = NULL;
        index++;
        return ;
    }

    p = strsep(&arg, " ");
    while (p) {
        if (strlen(p) == 0) {
            p = strsep(&arg, " ");
            continue;
        }
        parg[index] = p;
        index++;
        if (index > MAX_ARGS - 1) {
            break;
        }
        p = strsep(&arg, " ");
    }
    parg[index] = NULL;
}

/*
 * @cmd_str[in]
 * @cmd[out]: cmd string
 * @arg[out]: arg string
 * */
static void pre_handle_cmd_str(char *cmd_str, char **pcmd, char *parg[])
{
    char *cmd = NULL, *arg = NULL;
    int i = 0;

    if (strlen(cmd_str) == 0) {
        *pcmd = NULL;
        parg[0] = NULL;
        return;
    }

    if (cmd_str[strlen(cmd_str)-1] == '\n') {
        cmd_str[strlen(cmd_str)-1] = '\0';
    }

    /* skip the space at begining */
    for (; *cmd_str == ' '; cmd_str++);
    /* blank cmd */
    if (*cmd_str == 0) {
        *pcmd = NULL;
        parg[0] = NULL;
        return;
    }

    if ((arg = strchr(cmd_str, ' ')) == NULL) {
        cmd = cmd_str;
        parg[1] = NULL;
    } else {
        cmd = cmd_str;
        *arg = '\0';
        arg++;
    }

    *pcmd = cmd;
     split_args(arg, parg);
}

static void handle_cd_cmd(char *cmd, char *arg[])
{
    int ret = 0;

    if (arg[1] == NULL) {
        ret = chdir(get_user_home());
        if (ret < 0) {
            fprintf(stderr, "minish: cd: %s: No such file or directory\n", get_user_home());
        }
    } else {
        ret = chdir(arg[1]);
        if (ret < 0) {
            fprintf(stderr, "minish: cd: %s: No such file or directory\n", arg[1]);
        }
    }
}

/*
 * @cmd[in]: cmd of user input
 * @absolute_path[out]: absolute path of cmd
 * */
static int get_cmd_absolute_path(char *cmd, char *absolute_path)
{
    char path[CMDLINE_MAXLENGTH] = {0};
    char *tmp = path;
    char *p = NULL;
    struct stat sbuf;
    strcpy(path, getenv("PATH"));

    if (*cmd == '/') {
        strcpy(absolute_path, cmd);
        return 0;
    } else if (*cmd == '.' && *(cmd + 1) == '/') {
        getcwd(absolute_path, CMDLINE_MAXLENGTH);
        cmd++;
        strcat(absolute_path, cmd);
        return 0;
    } else {
        p = strsep(&tmp, ":");
        while (p) {
            strcpy(absolute_path, p);
            strcat(absolute_path, "/");
            strcat(absolute_path, cmd);
            if (stat(absolute_path, &sbuf) == 0) {
                return 0;
            }
            p = strsep(&tmp, ":");
        }

        return -1;
    }

    return -1;
}

static void fork_and_exec_cmd(char *cmd, char *arg[])
{
    pid_t pid = -1;
    int ret = 0;
    char *arg0 = NULL;
    char absolute_path[NAMELEN] = {0};

    ret = get_cmd_absolute_path(cmd, absolute_path);
    if (ret < 0) {
        fprintf(stderr, "minish: %s: command not found, get_cmd_absolute_path error\n", cmd);
        cmd_return_value = -127;
        return;
    }

    if ((pid = fork()) < 0) {
        fprintf(stderr, "minish: %s: command not found, fork error\n", cmd);
    } else if (pid > 0) {
        ret = waitpid(pid, &cmd_return_value, 0);
        if (ret < 0) {
            fprintf(stderr, "minish: %s: command not found, waitpid error\n", cmd);
        }
    } else {
        if ((arg0 = strrchr(absolute_path, '/')) != NULL) {
            arg0 ++;
        } else {
            arg0 = absolute_path;
        }
        arg[0] = arg0;

        ret = execv(absolute_path, arg);
        if (ret < 0) {
            fprintf(stderr, "minish: %s: command not found, exec error, errno: %d\n", cmd, errno);
        }
        exit(0);
    }
}

static void handle_cmd_return_value(char *arg[], char *cmd_ret)
{
    int index = 1;
    char *tmp = arg[index];

    while (tmp) {
        if (strcmp(tmp, "$?") == 0) {
            arg[index] = cmd_ret;
        }
        index++;
        tmp = arg[index];
    }
}

static int exec_cmd(char *cmd_str)
{
    char *cmd = NULL;
    char *arg[MAX_ARGS] = {0};
    int ret = 0;
    char cmd_ret[NAMELEN] = {0};
    sprintf(cmd_ret, "%d", WEXITSTATUS(cmd_return_value));

    pre_handle_cmd_str(cmd_str, &cmd, arg);
    handle_cmd_return_value(arg, cmd_ret);

    if (cmd == NULL) {
        return 0;
    }

    if (!strcmp(cmd, "cd")) {
        handle_cd_cmd(cmd, arg);
    } else {
        fork_and_exec_cmd(cmd, arg);
    }

    return 0;
}

static int start_minish(void)
{
    int ret = 0;
    char cmd_str[CMDLINE_MAXLENGTH] = {0};

    do {
        ret = exec_cmd(cmd_str);

        print_prompt();
    } while (myread(cmd_str, CMDLINE_MAXLENGTH) >= 0);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = init_signal_handler();
    if (ret < 0) {
        return -1;
    }

    ret = init_history_cmd();
    if (ret < 0) {
        return -1;
    }

    start_minish();

    ret = destroy_history_cmd();
    if (ret < 0) {
        return -1;
    }

    return 0;
}
