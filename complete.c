#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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

/*
 * just handle commands without path.
 * return value: the number of candidate commands.
 */
int complete_executable_cmd(char *buf)
{
    char path[CMDLINE_MAXLENGTH] = {0};
    char *p, *tmp = path;
    int candidate_num = 0;

    strcpy(path, getenv("PATH"));
    p = strsep(&tmp, ":");
    while (p) {
        candidate_num = search_candidate_cmd(buf, p, candidate_num);
        p = strsep(&tmp, ":");
    }

    if (candidate_num == 0) {
        /* do nothing */
    } else if (candidate_num == 1) { /* direct complete the command */
        strcpy(buf, candidate_cmds[0]);
    } else {
        print_list(candidate_cmds, candidate_num);
    }

    return candidate_num;
}

int complete_cmd_with_path(char *buf)
{
    int candidate_num = 0;
    char dir[NAMELEN] = {0};
    char *p = strrchr(buf, '/');

    if (p) {
        strncpy(dir, buf, p - buf + 1); /* include the '/' */
        p++;
        candidate_num = search_candidate_cmd(p, dir, candidate_num);
    } else {
        p = buf;
        candidate_num = search_candidate_cmd(p, "./", candidate_num);
    }

    if (candidate_num == 0) {
        /* nothing */
    } else if (candidate_num == 1) {
        strcpy(p, candidate_cmds[0]);
    } else {
        print_list(candidate_cmds, candidate_num);
    }

    return candidate_num;
}
