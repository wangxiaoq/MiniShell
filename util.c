#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>

inline char *get_current_user(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_name;
    }
}

inline char *get_user_home(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        return NULL;
    } else {
        return pw->pw_dir;
    }
}
