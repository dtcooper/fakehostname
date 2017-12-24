#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <time.h>

#define HOSTNAME_BUFSIZE 100
#define TIMESTR_BUFSIZE 20

char *call_uname() {
    static struct utsname buf;
    if (uname(&buf) == 0) {
        return buf.nodename;
    } else {
        printf("Error calling uname(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

char *call_gethostname() {
    static char hostname[HOSTNAME_BUFSIZE];
    hostname[HOSTNAME_BUFSIZE - 1] = '\0';

    if (gethostname(hostname, HOSTNAME_BUFSIZE - 1) == 0) {
        return hostname;
    } else {
        printf("Error calling gethostname(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

char *call_time() {
    static char timestr[TIMESTR_BUFSIZE];
    snprintf(timestr, TIMESTR_BUFSIZE, "%lld", (long long) time(NULL));
    return timestr;
}

int main(int argc, char **argv) {
    int n = 1;

    if (argc >= 2) {
        char *arg = argv[1];
        if (strcmp(arg, "uname") == 0) {
            printf("%s\n", call_uname());
        } else if (strcmp(arg, "gethostname") == 0) {
            printf("%s\n", call_gethostname());
        } else if (strcmp(arg, "time") == 0) {
            printf("%s\n", call_time());
        } else if (strcmp(arg, "all") == 0) {
            goto contd;
        } else {
            n = atoi(argv[1]);
            goto contd;
        }
    } else {
contd:
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                printf("uname(): %s\n", call_uname());
                printf("gethostname(): %s\n", call_gethostname());
                printf("time(): %s\n", call_time());
            }
        } else
            printf("Usage: %s [uname | gethostname | time | all | n]\n", argv[0]);
    }

    return EXIT_SUCCESS;
}
