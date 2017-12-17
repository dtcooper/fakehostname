#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
    #include <sys/syslimits.h>
#else
    #include <linux/limits.h>
#endif
#include <unistd.h>

#ifdef __APPLE__
    #define ENV_VARNAME_PRELOAD "DYLD_INSERT_LIBRARIES"
    #define ENV_PRELOAD_PATH_SEP ":"
    #define LIB_SUFFIX "dylib"
#else
    #define ENV_VARNAME_PRELOAD "LD_PRELOAD"
    #define ENV_PRELOAD_PATH_SEP " "
    #define LIB_SUFFIX "so"
#endif

#ifndef ENV_VARNAME_FAKE_HOSTNAME
    #define ENV_VARNAME_FAKE_HOSTNAME "FAKE_HOSTNAME"
#endif
#ifndef LIB_LOCATIONS
    #define LIB_LOCATIONS ".:/usr/local/lib:/usr/lib"
#endif
#ifndef LIB_FILENAME
    #define LIB_FILENAME "libfakehostname." LIB_SUFFIX
#endif

static char *custom_lib_path = NULL;
static char *new_hostname;
static int verbose = 0;

#define VERBOSE(...) if (verbose) printf(__VA_ARGS__)
#define SETENV(name, val, ...) \
    if (verbose) \
        printf("Setting environment variable \"%s\": \"%s\"\n", name, val); \
        setenv(name, val, __VA_ARGS__)

char *get_lib_path() {
    static char lib_path[PATH_MAX];
    char lib_locations[] = LIB_LOCATIONS;

    if (custom_lib_path != NULL) {
        VERBOSE("Using custom library path: %s\n", custom_lib_path);
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
        VERBOSE("Checking %s: ", lib_path);

        if (access(lib_path, X_OK) != -1) {
            VERBOSE("exists!\n");
            return lib_path;
        }
        VERBOSE("not found.\n");
    }

    printf("Couldn't find required library: " LIB_FILENAME "\n");
    exit(EXIT_FAILURE);
}


void usage(char *cmd_name, int exit_code) {
    printf(
        "Usage: %s [-v] [-l /path.to/lib." LIB_SUFFIX
        "] <new-hostname> <cmd> [<args> ...]\n", cmd_name);
    exit(exit_code);
}


int parse_options(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {"library", required_argument, 0, 'l'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "+vhl:", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                verbose = 1;
                break;
            case 'l':
                custom_lib_path = strdup(optarg);
                break;
            case 'h':
                usage(argv[0], EXIT_SUCCESS);
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

    new_hostname = argv[optind];
    VERBOSE("Faking hostname: %s\n", new_hostname);
    return optind + 1;
}

int main(int argc, char **argv) {
    int argv_cmd = parse_options(argc, argv);

    if (argc < 3) {
        printf("Usage %s <fake-hostname> <cmd> [<args...>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char preload_env_var_value[PATH_MAX];
    char *lib_path = get_lib_path();

    strncpy(preload_env_var_value, lib_path, PATH_MAX - 1);

    char *current_preload_env_var_value = getenv(ENV_VARNAME_PRELOAD);
    if (current_preload_env_var_value != NULL) {
        VERBOSE("Found existing value in " ENV_VARNAME_PRELOAD ": %s\n",
            current_preload_env_var_value);

        strncat(preload_env_var_value, ENV_PRELOAD_PATH_SEP,
            PATH_MAX - 1 - strlen(preload_env_var_value));

        strncat(preload_env_var_value,
                current_preload_env_var_value,
                PATH_MAX - 1 - strlen(preload_env_var_value));
    }
    preload_env_var_value[PATH_MAX - 1] = '\0';

    //Set FAKE_HOSTNAME, LD_PRELOAD (or DYLD vars for macOS)
    SETENV(ENV_VARNAME_FAKE_HOSTNAME, new_hostname, 1);
    SETENV(ENV_VARNAME_PRELOAD, preload_env_var_value, 1);
#ifdef __APPLE__
    SETENV("DYLD_FORCE_FLAT_NAMESPACE", "1", 1);
#endif

    if (verbose) {
        printf("exec():");
        for (int i = argv_cmd; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    int retval = execvp(argv[argv_cmd], &argv[argv_cmd]);
    if (retval == -1) {
        printf("Couldn't execute command \"%s\": %s\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
