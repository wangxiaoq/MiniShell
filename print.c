#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "util.h"

static int get_screen_width()
{//get terminal's width  
    struct winsize size;

    if(isatty(STDOUT_FILENO)==0)
    {
        return -1;
    }

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size)<0)
    {
        return -1;
    }

    return size.ws_col;
}

int print_list(char pTabl[][NAMELEN], int num)
{//list info with specified indent  
    int max_len = 0;
    int i = 0;
    int one_line_items = 0;
    int col_width = get_screen_width();

    for (i = 0; i < num; i++) {
        if (max_len < strlen(pTabl[i])) {
            max_len = strlen(pTabl[i]);
        }
    }

    one_line_items  = col_width/(max_len+4);

    printf("\n");
    for (i = 0; i < num; i++) {
        printf("%-*s", max_len+4, pTabl[i]);
        if ((i+1) % one_line_items == 0) {
            printf("\n");
        }
    }
    if (i % one_line_items != 0) {
        printf("\n");
    }

    return 0;
}

