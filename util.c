#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"

char *get_current_user(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_name;
    }
}

char *get_user_home(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_dir;
    }
}

void print_prompt(void)
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
    fprintf(stdout, "\r%s", prompt);
}

int is_dir(char *file)
{
    struct stat buf;

    if (stat(file, &buf) < 0) {
        return -1;
    }

    if (S_ISDIR(buf.st_mode)) {
        return 1;
    } else {
        return 0;
    }
}
