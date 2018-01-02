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

#define ACTION_DEFAULT (1 << 0)
#define ACTION_UNAME (1 << 1)
#define ACTION_GETHOSTNAME (1 << 2)
#define ACTION_TIME (1 << 3)
#define ACTION_QUIET (1 << 4)

#define ARG_EQUAL(i, x) (strcmp(x, argv[i]) == 0)
#define ACTION_ENABLE(a) action = (action & ~ACTION_DEFAULT) | a;
#define ACTION_IS_ENABLED_OFF_BY_DEFAULT(a) ((action & a) > 0)
#define ACTION_IS_ENABLED(a) (((action & ACTION_DEFAULT) | (action & a)) > 0)
#define VERBOSE(...) if (!((action & ACTION_QUIET) > 0)) printf(__VA_ARGS__)
#define DO_CALL(description, call) \
    { \
        if (!((action & ACTION_QUIET) > 0)) \
            printf(description "(): "); \
        printf("%s\n", call); \
    }

int main(int argc, char **argv) {
    int i;
    int n = 1;
    char action = ACTION_DEFAULT;

    for (i = 1; i < argc; i++) {
        int tmp = atoi(argv[i]);

        if (tmp > 0 ) {
            n = tmp;
        } else if (ARG_EQUAL(i, "uname"))
            ACTION_ENABLE(ACTION_UNAME)
        else if (ARG_EQUAL(i, "gethostname"))
            ACTION_ENABLE(ACTION_GETHOSTNAME)
        else if (ARG_EQUAL(i, "time"))
            ACTION_ENABLE(ACTION_TIME)
        else if (ARG_EQUAL(i, "quiet")) {
            action |= ACTION_QUIET;
        } else if (ARG_EQUAL(i, "all")) {
            ACTION_ENABLE(ACTION_UNAME)
            ACTION_ENABLE(ACTION_GETHOSTNAME)
            ACTION_ENABLE(ACTION_TIME)
        } else {
            printf("Usage: %s [uname] [gethostname] [time] [all] [n] [quiet]\n", argv[0]);
            return EXIT_SUCCESS;
        }
    }

    for (i = 0; i < n; i++) {
        if (n > 1)
            VERBOSE("------- Call #%d -------\n", i + 1);
        if (ACTION_IS_ENABLED(ACTION_UNAME))
            DO_CALL("uname", call_uname());
        if (ACTION_IS_ENABLED(ACTION_GETHOSTNAME))
            DO_CALL("gethostname", call_gethostname());
        if (ACTION_IS_ENABLED_OFF_BY_DEFAULT(ACTION_TIME))
            DO_CALL("time", call_time());
    }

    return EXIT_SUCCESS;
}
