#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/utsname.h>


#ifndef ENV_VARNAME_FAKE_HOSTNAME
    #define ENV_VARNAME_FAKE_HOSTNAME "FAKE_HOSTNAME"
#endif

static int (*__orig_gethostname)(char *name, size_t len) = NULL;
static int (*__orig_uname)(struct utsname *buf) = NULL;


int gethostname(char *name, size_t len) {
    int retval;
    char *fake_hostname;

    if (__orig_gethostname == NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        __orig_gethostname = dlsym(RTLD_NEXT, "gethostname");
    }
#pragma GCC diagnostic pop

    fake_hostname =  getenv(ENV_VARNAME_FAKE_HOSTNAME);

    if (fake_hostname) {
        strncpy(name, fake_hostname, len - 2);
        name[len - 1] = '\0';
        retval = 0;
    } else {
        retval = __orig_gethostname(name, len);
    }

    return retval;
}

int uname(struct utsname *buf) {
    int retval;
    char *fake_hostname;

    if (__orig_uname == NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        __orig_uname = dlsym(RTLD_NEXT, "uname");
#pragma GCC diagnostic pop
    }

    fake_hostname = getenv(ENV_VARNAME_FAKE_HOSTNAME);
    retval = __orig_uname(buf);

    if (fake_hostname) {
        strncpy(buf->nodename, fake_hostname, sizeof(buf->nodename) - 2);
        buf->nodename[sizeof(buf->nodename) - 1] = '\0';
    }

    return retval;
}
