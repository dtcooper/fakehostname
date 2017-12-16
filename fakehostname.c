#ifdef __APPLE__
    #include <limits.h>
#else
    #include <linux/limits.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ENV_PATH_VAL_MAXLEN 1024
#ifndef ENV_VARNAME
    #define ENV_VARNAME "FAKE_HOSTNAME"
#endif

#ifndef LIB_LOCATIONS
    #define LIB_LOCATIONS ".:/usr/lib:/usr/local/lib"
#endif
#ifndef LIB_FILENAME
    #ifdef __APPLE__
        #define LIB_FILENAME "libfakehostname.dylib"
    #else
        #define LIB_FILENAME "libfakehostname.so"
    #endif
#endif

#ifdef __APPLE__
    #define ENV_PATH_NAME "DYLD_INSERT_LIBRARIES"
#else
    #define ENV_PATH_NAME "LD_PRELOAD"
#endif

static char lib_path[PATH_MAX + 1];

char *get_lib_path()
{
    static char *lib_prefixes = LIB_LOCATIONS;
    int j = 0;

    for (unsigned int i = 0; i < sizeof(LIB_LOCATIONS); i++) {
        lib_path[j] = lib_prefixes[i];
        if (lib_prefixes[i] == ':' || lib_prefixes[i] == '\0') {
            lib_path[j] = '\0';
            strncat(lib_path, "/" LIB_FILENAME, PATH_MAX);
            if (access(lib_path, X_OK) != -1) {
                return lib_path;
            }
            j = 0;
        } else {
            j++;
        }
    }
    printf("Couldn't find library: " LIB_FILENAME "\n");
    exit(1);
}


int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage %s <fakehostname> <cmd> [<args...>]\n", argv[0]);
        return 1;
    }

    char path_env_val[ENV_PATH_VAL_MAXLEN + 1] = "";
    char *lib_path = get_lib_path();

    strncat(path_env_val, lib_path, ENV_PATH_VAL_MAXLEN);

    char *current_path_env_val = getenv(ENV_PATH_NAME);
    if (current_path_env_val != NULL) {
        strncat(path_env_val, current_path_env_val, ENV_PATH_VAL_MAXLEN);
        strncat(
            path_env_val,
#ifdef __APPLE__
            ":",
#else
            " ",
#endif
            ENV_PATH_VAL_MAXLEN);
    }

    if (strlen(path_env_val) >= (ENV_PATH_VAL_MAXLEN - 1)) {
        printf("Environment variable " ENV_PATH_NAME " too long. Exiting.\n");
        return 1;
    }

    setenv(ENV_VARNAME, argv[1], 1);

    setenv(ENV_PATH_NAME, path_env_val, 1);
#ifdef __APPLE__
    setenv("DYLD_FORCE_FLAT_NAMESPACE", "1", 1);
#endif
    printf("%s\n", getenv(ENV_PATH_NAME));
    execvp(argv[2], &argv[2]);
}
