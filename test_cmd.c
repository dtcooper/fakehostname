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

int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "uname") == 0) {
            do_uname();
        } else if (strcmp(argv[1], "gethostname") == 0) {
            do_gethostname();
        }
    } else {
        printf("uname(): ");
        do_uname();
        printf("gethostname(): ");
        do_gethostname();
    }

    return 0;
}
