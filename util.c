#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>


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
