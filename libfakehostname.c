#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/utsname.h>


#define ENV_VARNAME "FAKE_HOSTNAME"


static int (*__orig_gethostname)(char *name, size_t len) = NULL;
static int (*__orig_uname)(struct utsname *buf) = NULL;


int gethostname(char *name, size_t len) {
    int retval;
    char *fake_hostname;

    if (__orig_gethostname == NULL) {
        __orig_gethostname = dlsym(RTLD_NEXT, "gethostname");
    }

    fake_hostname =  getenv(ENV_VARNAME);

    if (fake_hostname) {
        strncpy(name, fake_hostname, len);
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
        __orig_uname = dlsym(RTLD_NEXT, "uname");
    }

    fake_hostname = getenv(ENV_VARNAME);
    retval = __orig_uname(buf);

    if (fake_hostname) {
        strncpy(buf->nodename, fake_hostname, sizeof(buf->nodename));
    }

    return retval;
}
