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

#include "common.h"

static char *custom_lib_path = NULL;
static char *new_hostname;

#ifdef ENABLE_VERBOSE
    static int verbose = 0;
    #define VERBOSE(...) \
        if (verbose) \
            fprintf(stderr, "fakehostname: " __VA_ARGS__);
#else
    #define VERBOSE(...)
#endif
#define SETENV(name, val, ...) \
    VERBOSE("Setting environment variable \"%s\": \"%s\"\n", name, val) \
    setenv(name, val, __VA_ARGS__);

char *get_lib_path() {
    static char lib_path[PATH_MAX];
    char lib_locations[] = LIB_LOCATIONS;

    if (custom_lib_path != NULL) {
        VERBOSE("Using custom library path: %s\n", custom_lib_path)
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
            VERBOSE("Checking %s: exists!\n", lib_path)
            return lib_path;
        }
        VERBOSE("Checking %s: not found.\n", lib_path)
    }

    printf("Couldn't find required library: " LIB_FILENAME "\n");
    exit(EXIT_FAILURE);
}


void usage(char *cmd_name, int exit_code) {
    printf(
        "fakehostname version " FAKE_HOSTNAME_VERSION "\n\n"
        "Usage: %s [-h"
#ifdef ENABLE_VERBOSE
        "v"
#endif
        "V] [-l ./lib." LIB_SUFFIX "] <new-hostname> <cmd> [<args> ...]\n\n"
        "This command fakes your system's hostname for a given command, overriding libc\n"
        "calls to uname() and gethostname()\n\n"
        "Positional arguments:\n"
        "  <new-hostname>      Fake hostname to use\n"
        "  <cmd> [<args> ...]  Command and its arguments to execute\n\n"
        "Optional arguments:\n"
        "  -h, --help          Show this help message and exit\n"
        "  -l /abs.path/to/mylib." LIB_SUFFIX ", --library /abs.path/to/mylib." LIB_SUFFIX "\n"
        "                      Custom path of fakehostname library (must be absolute)\n"
#ifdef ENABLE_VERBOSE
        "  -v, --verbose       Print verbose/debug output to stderr\n"
#endif
        "  -V, --version       Print version information and exit\n\n"
        "...and remember kids, have fun!\n\n",
    basename(cmd_name));
    exit(exit_code);
}

int parse_options(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"library", required_argument, 0, 'l'},
#ifdef ENABLE_VERBOSE
      {"verbose", no_argument, 0, 'v'},
#endif
      {"version", no_argument, 0, 'V'},
      {0, 0, 0, 0}
    };

    while ((c = getopt_long(
        argc, argv,
        "+hl:"
#ifdef ENABLE_VERBOSE
        "v"
#endif
        "V",
        long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                usage(argv[0], EXIT_SUCCESS);
                break;
            case 'l':
                custom_lib_path = strdup(optarg);
                break;
#ifdef ENABLE_VERBOSE
            case 'v':
                verbose = 1;
                break;
#endif
            case 'V':
                printf("fakehostname version " FAKE_HOSTNAME_VERSION "\n");
                exit(EXIT_SUCCESS);
            case '?':
            default:
                usage(argv[0], EXIT_FAILURE);
                break;
       }
    }
    if ((optind + 2) > argc) {
        usage(argv[0], EXIT_FAILURE);
    }

    new_hostname = argv[optind];
    VERBOSE("Faking hostname: %s\n", new_hostname)
    return optind + 1;
}

int main(int argc, char **argv) {
    int argv_cmd = parse_options(argc, argv);

    char preload_env_var_value[PATH_MAX];
    char *lib_path = get_lib_path();

    strncpy(preload_env_var_value, lib_path, PATH_MAX - 1);

    char *current_preload_env_var_value = getenv(ENV_VARNAME_PRELOAD);
    if (current_preload_env_var_value != NULL) {
        VERBOSE("Found existing value in " ENV_VARNAME_PRELOAD ": %s\n",
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

#ifdef ENABLE_VERBOSE
    if (verbose) {
        SETENV(ENV_VARNAME_ENABLE_VERBOSE, "1", 1)
        VERBOSE("exec():")
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
