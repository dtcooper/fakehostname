#define _GNU_SOURCE

#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#ifdef ENABLE_VERBOSE
    #include <stdio.h>
#endif
#include <sys/utsname.h>

#include "common.h"

static int (*__orig_gethostname)(char *name, size_t len) = NULL;
static int (*__orig_uname)(struct utsname *buf) = NULL;

char *version = FAKE_HOSTNAME_VERSION;

#ifdef ENABLE_VERBOSE
    static int verbose = -1;
    #define VERBOSE(...) \
    if (verbose == -1) { \
        verbose = ((getenv(ENV_VARNAME_ENABLE_VERBOSE) == NULL) ? 0 : 1); \
        if (verbose) { \
            fprintf(stderr, "libfakehostname: Version %s\n", version); \
            fprintf(stderr, "libfakehostname: \"" ENV_VARNAME_ENABLE_VERBOSE \
                            "\" set. Enabling verbose mode.\n"); \
        } \
    } \
    if (verbose) \
        fprintf(stderr, "libfakehostname: " __VA_ARGS__);
#else
    #define VERBOSE(...)
#endif


int gethostname(char *name, size_t len) {
    int retval;
    char *fake_hostname = getenv(ENV_VARNAME_FAKE_HOSTNAME);

    if ((fake_hostname != NULL) && (fake_hostname[0] != '\0')) {
        strncpy(name, fake_hostname, len - 2);
        name[len - 1] = '\0';
        VERBOSE("gethostname(): Faked to \"%s\"\n", name)
        retval = 0;
    } else {
        VERBOSE("gethostname(): Environment variable \"" ENV_VARNAME_FAKE_HOSTNAME
                "\" empty. Can't fake hostname.\n")

        if (__orig_gethostname == NULL) {
            VERBOSE("First call to gethostname(), calling dlsym(RTLD_NEXT, \"gethostname\")\n")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
            __orig_gethostname = dlsym(RTLD_NEXT, "gethostname");
        }
#pragma GCC diagnostic pop

        retval = __orig_gethostname(name, len);
    }

    return retval;
}


int uname(struct utsname *buf) {
    int retval;

    if (__orig_uname == NULL) {
        VERBOSE("First call to uname(), calling dlsym(RTLD_NEXT, \"uname\")\n")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        __orig_uname = dlsym(RTLD_NEXT, "uname");
#pragma GCC diagnostic pop
    }

    retval = __orig_uname(buf);
    char *fake_hostname = getenv(ENV_VARNAME_FAKE_HOSTNAME);

    if ((fake_hostname != NULL) && (fake_hostname[0] != '\0')) {
        strncpy(buf->nodename, fake_hostname, sizeof(buf->nodename) - 2);
        buf->nodename[sizeof(buf->nodename) - 1] = '\0';
        VERBOSE("uname(): Faked to \"%s\"\n", buf->nodename)
    }
#ifdef ENABLE_VERBOSE
    else {
        VERBOSE("uname(): No environment variable \"" ENV_VARNAME_FAKE_HOSTNAME
                "\" exists. Can't fake hostname.\n")
    }
#endif

    return retval;
}
