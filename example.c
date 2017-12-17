#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#define HOSTNAME_BUFSIZE 1024

void do_uname() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        printf("%s\n", buf.nodename);
    } else {
        printf("Error calling uname(): %s\n", strerror(errno));
    }
}

void do_gethostname() {
    char hostname[HOSTNAME_BUFSIZE];
    hostname[HOSTNAME_BUFSIZE - 1] = '\0';

    if (gethostname(hostname, HOSTNAME_BUFSIZE - 1) == 0) {
        printf("%s\n", hostname);
    } else {
        printf("Error calling gethostname(): %s\n", strerror(errno));
    }
}

void do_both() {
    printf("uname(): ");
    do_uname();
    printf("gethostname(): ");
    do_gethostname();
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        char *arg = argv[1];
        if (strcmp(arg, "uname") == 0)
            do_uname();
        else if (strcmp(arg, "gethostname") == 0)
            do_gethostname();
        else if (strcmp(arg, "all") == 0)
            do_both();
        else
            printf("Usage: %s [uname | gethostname | all]\n", argv[0]);
    } else
        do_both();

    return 0;
}
