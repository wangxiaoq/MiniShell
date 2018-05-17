#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

static void sigint_handler(int signo)
{
    fprintf(stdout, "^C");
    fflush(stdout);
    return ;
}

static void sigquit_handler(int signo)
{
    return ;
}

int init_signal_handler(void)
{
    struct sigaction sigact;
    int ret = 0;
    memset(&sigact, 0, sizeof(sigact));

    sigact.sa_handler = sigint_handler;
    sigemptyset(&sigact.sa_mask);
    sigaddset(&sigact.sa_mask, SIGINT);
    sigaddset(&sigact.sa_mask, SIGQUIT);
    ret = sigaction(SIGINT, &sigact, NULL);
    if (ret < 0) {
        return -1;
    }

    sigact.sa_handler = sigquit_handler;
    sigemptyset(&sigact.sa_mask);
    sigaddset(&sigact.sa_mask, SIGQUIT);
    ret = sigaction(SIGQUIT, &sigact, NULL);
    if (ret < 0) {
        return -1;
    }

    return 0;
}
