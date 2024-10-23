#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>

#include "fakehostname.h"

static int (*__orig_gethostname)(char *name, size_t len) = NULL;
static int (*__orig_uname)(struct utsname *buf) = NULL;

char *version = FAKE_HOSTNAME_VERSION
#ifdef ENABLE_DEBUG
    " [debug enabled]"
#endif
;

static void __attribute__((constructor)) initializer()  {
#ifdef ENABLE_DEBUG
    SET_DEBUG_FROM_ENV()
    DEBUG("Version %s\n", version)
#endif

    DEBUG("Calling dlsym() for \"uname\" and \"gethostname\".\n")

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    __orig_gethostname = dlsym(RTLD_NEXT, "gethostname");
    __orig_uname = dlsym(RTLD_NEXT, "uname");
#pragma GCC diagnostic pop

    if ((__orig_gethostname == NULL) || (__orig_uname == NULL)) {
        puts(dlerror());
        exit(EXIT_FAILURE);
    }
}

int gethostname(char *name, size_t len) {
    int retval;
    char *fake_hostname = getenv(ENV_VARNAME_FAKE_HOSTNAME);

    if ((fake_hostname != NULL) && (fake_hostname[0] != '\0')) {
        strncpy(name, fake_hostname, len - 2);
        name[len - 1] = '\0';
        DEBUG("gethostname(): Faked to \"%s\"\n", name)
        retval = 0;
    } else {
        DEBUG("gethostname(): Environment variable \"" ENV_VARNAME_FAKE_HOSTNAME
                "\" empty. Can't fake hostname.\n")

        retval = __orig_gethostname(name, len);
    }

    return retval;
}


int uname(struct utsname *buf) {
    int retval;

    retval = __orig_uname(buf);
    char *fake_hostname = getenv(ENV_VARNAME_FAKE_HOSTNAME);

    if ((fake_hostname != NULL) && (fake_hostname[0] != '\0')) {
        strncpy(buf->nodename, fake_hostname, sizeof(buf->nodename) - 2);
        buf->nodename[sizeof(buf->nodename) - 1] = '\0';
        DEBUG("uname(): Faked to \"%s\"\n", buf->nodename)
    }
#ifdef ENABLE_DEBUG
    else {
        DEBUG("uname(): Environment variable \"" ENV_VARNAME_FAKE_HOSTNAME
                "\" empty. Can't fake hostname.\n")
    }
#endif

    return retval;
}
