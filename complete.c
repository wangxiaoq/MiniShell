#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "util.h"
#include "print.h"

#define MAX_CMD_NUM 2048

static char candidate_cmds[MAX_CMD_NUM][NAMELEN];

int is_sys_executable_cmd(char *buf)
{
    if (!strchr(buf, '/')) {
        return 1;
    }
    return 0;
}

int is_sudo_cmd(char *buf)
{
    int i = 0, space_num = 0;
    int length = strlen(buf);
    char *p = buf;

    while (*p == ' ') {
        p++;
    }

    if (strncmp(p, "sudo", strlen("sudo"))) {
        return 0;
    }

    for (i = 0; i < length; i++) {
        if (i > 0 && p[i] == ' ' && p[i-1] != ' ') {
            space_num++;
        }
    }

    if (space_num == 1) {
        return 1;
    }

    return 0;
}

/* search candidate commands into `candidate_cmds` variable */
static int search_candidate_cmd(char *buf, char *dir_name, int index)
{
    DIR *dir = NULL;
    struct dirent *ptr = NULL;

    dir = opendir(dir_name);
    if (dir == NULL) { /* skip the open directory error */
        return index;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if ((!strcmp(".", ptr->d_name)) || (!strcmp("..", ptr->d_name))) {
            continue;
        }
        if (strncmp(buf, ptr->d_name, strlen(buf)) == 0) {
            strcpy(candidate_cmds[index], ptr->d_name);
            if (ptr->d_type == 4) {
                strcat(candidate_cmds[index], "/");
            }
            index++;
        }
    }

    return index;
}

static int del_repeated_cmds(int candidate_num)
{
    int i = 0, j = 0, orig_num = candidate_num;

    for (i = 0; i < orig_num; i++) {
        for (j = i + 1; j < orig_num; j++) {
            if ((strlen(candidate_cmds[i]) != 0) && (!strcmp(candidate_cmds[i], candidate_cmds[j]))) {
                memset(candidate_cmds[j], 0, NAMELEN);
                candidate_num--;
            }
        }
    }

    for (i = 0; i < orig_num; i++) {
        if (!strlen(candidate_cmds[i])) {
            for (j = i + 1; j < orig_num; j++) {
                if (strlen(candidate_cmds[j])) {
                    strcpy(candidate_cmds[i], candidate_cmds[j]);
                    memset(candidate_cmds[j], 0, NAMELEN);
                    break;
                }
            }
        }
    }

    return candidate_num;
}

static int has_common_substr(int candidate_num, char *substr)
{
    int i = 0, j = 0, min_len = 0, len = 0;
    if (candidate_num == 0 || candidate_num == 1) {
        return 0;
    }

    for (i = 0; i < candidate_num; i++) {
        len = strlen(candidate_cmds[i]);
        if (min_len == 0 || min_len > len) {
            min_len = len;
        }
    }

    for (i = 0; i < min_len; i++) {
        for (j = 1; j < candidate_num; j++) {
            if (candidate_cmds[j][i] != candidate_cmds[0][i]) {
                goto out;
            }
        }
        substr[i] = candidate_cmds[0][i];
    }

out:
    return (i == 0) ? 0 : 1;
}

/*
 * just handle commands without path.
 * return value: whether complete the commands or not.
 */
int complete_sys_cmd(char *buf, int tab_hit_times)
{
    char path[CMDLINE_MAXLENGTH] = {0};
    char substr[CMDLINE_MAXLENGTH] = {0};
    char *p, *tmp = path;
    int candidate_num = 0;
    int has_substr = 0;

    if (strlen(buf) == 0) {
        return 0;
    }

    strcpy(path, getenv("PATH"));
    p = strsep(&tmp, ":");
    while (p) {
        candidate_num = search_candidate_cmd(buf, p, candidate_num);
        p = strsep(&tmp, ":");
    }

    candidate_num = del_repeated_cmds(candidate_num);

    if (candidate_num == 0) {
        /* do nothing */
    } else if (candidate_num == 1) { /* direct complete the command */
        strcpy(buf, candidate_cmds[0]);
        strcat(buf, " ");
    } else {
        has_substr = has_common_substr(candidate_num, substr);
        if (has_substr && strcmp(buf, substr)) {
            strcpy(buf, substr);
        } else {
            if (tab_hit_times > 1) {
                print_list(candidate_cmds, candidate_num);
            }
        }
    }

    return (candidate_num == 1 || has_substr) ? 1 : 0;
}

static void handle_cmd_under_current_dir(int *candidate_num)
{
    int i = 0, j = 0;
    int orig_num = *candidate_num;
    for (i = 0; i < orig_num; i++) {
        if (access(candidate_cmds[i], X_OK) < 0) {
            memset(candidate_cmds[i], 0, NAMELEN);
            (*candidate_num)--;
        }
    }

    for (i = 0; i < orig_num; i++) {
        if (strlen(candidate_cmds[i]) == 0) {
            for (j = i+1; j < orig_num; j++) {
                if (strlen(candidate_cmds[j])) {
                    strcpy(candidate_cmds[i], candidate_cmds[j]);
                    memset(candidate_cmds[j], 0, NAMELEN);
                }
            }
        }
    }
}

int complete_cmd_with_path(char *buf, int is_arg, int tab_hit_times)
{
    int candidate_num = 0;
    char dir[NAMELEN] = {0};
    char substr[CMDLINE_MAXLENGTH] = {0};
    char *p = strrchr(buf, '/');
    int has_substr = 0;

    if (p) {
        strncpy(dir, buf, p - buf + 1); /* include the '/' */
        p++;
        candidate_num = search_candidate_cmd(p, dir, candidate_num);
    } else {
        p = buf;
        candidate_num = search_candidate_cmd(p, "./", candidate_num);
    }

    if (!strcmp("./", dir)) {
        if (!is_arg) {
            handle_cmd_under_current_dir(&candidate_num);
        }
    }

    candidate_num = del_repeated_cmds(candidate_num);

    if (candidate_num == 0) {
        /* nothing */
    } else if (candidate_num == 1) {
        strcpy(p, candidate_cmds[0]);
        if (!is_dir(buf)) {
            strcat(buf, " ");
        }
    } else {
        has_substr = has_common_substr(candidate_num, substr);
        if (has_substr && strcmp(p, substr)) {
            strcpy(p, substr);
        } else {
            if (tab_hit_times > 1) {
                print_list(candidate_cmds, candidate_num);
            }
        }
    }

    return (candidate_num == 1 || has_substr) ? 1 : 0;
}
