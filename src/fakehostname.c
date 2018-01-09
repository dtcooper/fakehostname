#include <dlfcn.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
    #include <sys/syslimits.h>
#else
    #include <linux/limits.h>
#endif
#include <unistd.h>

#define CFILENAME "fakehostname"
#include "common.h"

static char *custom_lib_path = NULL;
static char *new_hostname;

#define SETENV(name, val, ...) \
    DEBUG("Setting environment variable \"%s\": \"%s\"\n", name, val) \
    setenv(name, val, __VA_ARGS__);

char *get_lib_path() {
    static char lib_path[PATH_MAX];
    char lib_locations[] = LIB_LOCATIONS;

    if (custom_lib_path != NULL) {
        DEBUG("Using custom library path: %s\n", custom_lib_path)
        if (access(custom_lib_path, X_OK) == -1) {
            printf("No library found: %s\n", custom_lib_path);
            exit(EXIT_FAILURE);
        }

        return custom_lib_path;
    }

    for (
        char *lib_path_prefix = strtok(lib_locations, ":");
        lib_path_prefix != NULL;
        lib_path_prefix = strtok(NULL, ":")
    ) {
        strncpy(lib_path, lib_path_prefix, PATH_MAX - 1);
        strncat(lib_path, "/" LIB_FILENAME, PATH_MAX - 1 - strlen(lib_path));
        lib_path[PATH_MAX - 1] = '\0';

        if (access(lib_path, X_OK) != -1) {
            DEBUG("Checking %s: exists!\n", lib_path)
            return lib_path;
        }
        DEBUG("Checking %s: not found.\n", lib_path)
    }

    printf("Couldn't find required library: " LIB_FILENAME "\n");
    exit(EXIT_FAILURE);
}

void version(int exit_code) {
    printf("fakehostname version: " FAKE_HOSTNAME_VERSION "\n");

    SET_DEBUG(0) //disable potential debug output on get_lib_path
    char *lib_path = get_lib_path();
    void *lib_handle = dlopen(lib_path, RTLD_LAZY);
    if (!lib_handle) {
        printf("Error opening library %s: \"%s\"\n", lib_path, dlerror());
        return;
    }

    char **lib_version = dlsym(lib_handle, "version");

    char *error;
    if ((error = dlerror()) != NULL)  {
        printf("Error loading \"version\" from %s: \"%s\"\n", lib_path, error);
        return;
    }

    printf("libfakehostname version: %s (%s)\n", *lib_version, lib_path);
    exit(exit_code);
}

void usage(char *cmd_name, int exit_code) {
    printf(
        "\nUsage: %s [-"
#ifdef ENABLE_DEBUG
        "d"
#endif
        "hv] [-l ./lib." LIB_SUFFIX "] <new-hostname> <cmd> [<args> ...]\n\n"
        "This command fakes your system's hostname for a given command, overriding libc\n"
        "calls to uname() and gethostname()\n\n"
        "Positional arguments:\n"
        "  <new-hostname>      Fake hostname to use\n"
        "  <cmd> [<args> ...]  Command and its arguments to execute\n\n"
        "Optional arguments:\n"
#ifdef ENABLE_DEBUG
        "  -d, --debug         Print debug (verbose) output to stderr\n"
#endif
        "  -h, --help          Show this help message and exit\n"
        "  -l /abs.path/to/mylib." LIB_SUFFIX ", --library /abs.path/to/mylib." LIB_SUFFIX "\n"
        "                      Custom path of fakehostname library (must be absolute)\n"
        "  -v, --version       Print version information and exit\n\n"
        "...and remember kids, have fun!\n\n",
    basename(cmd_name));
    version(exit_code);
}

int parse_options(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
#ifdef ENABLE_DEBUG
      {"debug", no_argument, 0, 'd'},
#endif
      {"help", no_argument, 0, 'h'},
      {"library", required_argument, 0, 'l'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

    while ((c = getopt_long(
        argc, argv,
        "+"
#ifdef ENABLE_DEBUG
        "d"
#endif
        "hl:v",
        long_options, NULL)) != -1) {
        switch (c) {
#ifdef ENABLE_DEBUG
            case 'd':
                SET_DEBUG(1)
                break;
#endif
            case 'h':
                usage(argv[0], EXIT_SUCCESS);
                break;
            case 'l':
                custom_lib_path = strdup(optarg);
                break;
            case 'v':
                version(EXIT_SUCCESS);
                break;
            case '?':
            default:
                usage(argv[0], EXIT_FAILURE);
                break;
       }
    }
    if ((optind + 2) > argc) {
        usage(argv[0], EXIT_FAILURE);
    }

    DEBUG("Version: " FAKE_HOSTNAME_VERSION "\n");

    new_hostname = argv[optind];
    DEBUG("Faking hostname: %s\n", new_hostname)
    return optind + 1;
}

int main(int argc, char **argv) {
    int argv_cmd = parse_options(argc, argv);

    char preload_env_var_value[PATH_MAX];
    char *lib_path = get_lib_path();

    strncpy(preload_env_var_value, lib_path, PATH_MAX - 1);

    char *current_preload_env_var_value = getenv(ENV_VARNAME_PRELOAD);
    if (current_preload_env_var_value != NULL) {
        DEBUG("Found existing value in " ENV_VARNAME_PRELOAD ": %s\n",
            current_preload_env_var_value)

        strncat(preload_env_var_value, ENV_PRELOAD_PATH_SEP,
            PATH_MAX - 1 - strlen(preload_env_var_value));

        strncat(preload_env_var_value,
                current_preload_env_var_value,
                PATH_MAX - 1 - strlen(preload_env_var_value));
    }
    preload_env_var_value[PATH_MAX - 1] = '\0';

    //Set FAKE_HOSTNAME, LD_PRELOAD (or DYLD vars for macOS)
    SETENV(ENV_VARNAME_PRELOAD, preload_env_var_value, 1)
#ifdef __APPLE__
    SETENV("DYLD_FORCE_FLAT_NAMESPACE", "1", 1)
#endif

    SETENV(ENV_VARNAME_FAKE_HOSTNAME, new_hostname, 1)

#ifdef ENABLE_DEBUG
    if (_debug) {
        SETENV(ENV_VARNAME_ENABLE_DEBUG, "1", 1)
        DEBUG("exec():")
        for (int i = argv_cmd; i < argc; i++) {
            fprintf(stderr, " %s", argv[i]);
        }
        fprintf(stderr, "\n");
    }
#endif

    int retval = execvp(argv[argv_cmd], &argv[argv_cmd]);
    if (retval == -1) {
        printf("Couldn't execute command \"%s\": %s\n", argv[argv_cmd], strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
