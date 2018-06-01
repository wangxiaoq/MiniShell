/*
 * environment variable handling.
 */
#include <string.h>
#include <stdlib.h>

/*
 * judge wether the cmd is add environment variable.
 */
int is_add_environment_variable(char *cmd)
{
    if (strchr(cmd, '=')) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * add environment variable.
 */
int add_environment_variable(char *cmd)
{
    char *tmp = strsep(&cmd, "=");
    return setenv(tmp, cmd, 1);
}

/*
 * use environment variable.
 */
int handle_environment_variable(char *arg[])
{
    int i = 1;
    char *tmp = arg[i];

    while (tmp) {
        if (strchr(tmp, '$') == tmp) {
            arg[i] = getenv(tmp+1);
        }
        i++;
        tmp = arg[i];
    }
}
