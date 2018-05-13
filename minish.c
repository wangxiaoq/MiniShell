#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

#define CMDLINE_MAXLENGTH 4096
#define NAMELEN 512

static inline char *get_current_user(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_name;
    }
}

static inline char *get_user_home(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_dir;
    }
}

static inline int print_prompt(void)
{
    char prompt[NAMELEN] = {0};
    char hostname[NAMELEN] = {0};
    char current_dir[NAMELEN] = {0};
    int ret = 0;

    char *user = get_current_user();
    ret = gethostname(hostname, CMDLINE_MAXLENGTH);
    if (ret < 0) {
        memset(hostname, 0, CMDLINE_MAXLENGTH);
    }
    sprintf(prompt, "%s@%s:%s%s ", user, hostname, getcwd(current_dir, NAMELEN), !strcmp(user, "root") ? "#" : "$");
    fprintf(stdout, prompt);

    return 0;
}

/*
 * @cmd_str[in]
 * @cmd[out]: cmd string
 * @arg[out]: arg string
 * */
static void pre_handle_cmd_str(char *cmd_str, char **pcmd, char **parg)
{
    char *cmd = NULL, *arg = NULL;
    int i = 0;

    if (strlen(cmd_str) == 0) {
        *pcmd = NULL;
        *parg = NULL;
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
        *parg = NULL;
        return;
    }

    if ((arg = strchr(cmd_str, ' ')) == NULL) {
        cmd = cmd_str;
    } else {
        cmd = cmd_str;
        *arg = '\0';
        arg++;
        /* skip the space at beginning*/
        for (; *arg == ' '; arg++);
        if (*arg == 0) {
            arg = NULL;
        }

        /* skip the space at end */
        if (arg != NULL) {
            for (i = strlen(arg) - 1; arg[i] == ' '; i--) {
                arg[i] = 0;
            }
        }
    }

    *pcmd = cmd;
    *parg = arg;
}

static void handle_cd_cmd(char *cmd, char *arg)
{
    int ret = 0;

    if (arg == NULL) {
        ret = chdir(get_user_home());
        if (ret < 0) {
            fprintf(stderr, "jsh: cd: %s: No such file or directory\n", get_user_home());
        }
    } else {
        ret = chdir(arg);
        if (ret < 0) {
            fprintf(stderr, "jsh: cd: %s: No such file or directory\n", arg);
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

static void fork_and_exec_cmd(char *cmd, char *arg)
{
    pid_t pid = -1;
    int status = 0;
    int ret = 0;
    char *arg0 = NULL;
    char absolute_path[NAMELEN] = {0};

    ret = get_cmd_absolute_path(cmd, absolute_path);
    if (ret < 0) {
        fprintf(stderr, "jsh: %s: command not found, get_cmd_absolute_path error\n", cmd);
        return;
    }

    if ((pid = fork()) < 0) {
        fprintf(stderr, "jsh: %s: command not found, fork error\n", cmd);
    } else if (pid > 0) {
        ret = waitpid(pid, &status, 0);
        if (ret < 0) {
            fprintf(stderr, "jsh: %s: command not found, waitpid error\n", cmd);
        }
    } else {
        if ((arg0 = strrchr(absolute_path, '/')) != NULL) {
            arg0 ++;
        } else {
            arg0 = absolute_path;
        }

        ret = execl(absolute_path, arg0, arg, (char *)NULL);
        if (ret < 0) {
            fprintf(stderr, "jsh: %s: command not found, exec error, errno: %d\n", cmd, errno);
        }
        exit(0);
    }
}

static int exec_cmd(char *cmd_str)
{
    char *cmd = NULL, *arg = NULL;
    int ret = 0;

    pre_handle_cmd_str(cmd_str, &cmd, &arg);

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
    char cmd_str[CMDLINE_MAXLENGTH] = {0};
    int ret = 0;

    do {
        ret = exec_cmd(cmd_str);

        print_prompt();
    } while ((fgets(cmd_str, CMDLINE_MAXLENGTH, stdin)) != NULL);

    return 0;
}

int main(int argc, char *argv[])
{
    start_minish();
    return 0;
}
