#ifndef FAKEHOSTNAME_H_
    #define FAKEHOSTNAME_H_

// Apple or Linux configuration
#ifdef __APPLE__
    #define ENV_VARNAME_PRELOAD "DYLD_INSERT_LIBRARIES"
    #define ENV_PRELOAD_PATH_SEP ":"
    #define LIB_SUFFIX "dylib"
#else
    #define ENV_VARNAME_PRELOAD "LD_PRELOAD"
    #define ENV_PRELOAD_PATH_SEP " "
    #define LIB_SUFFIX "so"
#endif

// Defaults for compile-time variables
#ifndef ENV_VARNAME_FAKE_HOSTNAME
    #define ENV_VARNAME_FAKE_HOSTNAME "FAKE_HOSTNAME"
#endif

#ifndef ENV_VARNAME_ENABLE_DEBUG
    #define ENV_VARNAME_ENABLE_DEBUG "FAKE_HOSTNAME_ENABLE_DEBUG"
#endif

#ifndef LIB_LOCATIONS
    #define LIB_LOCATIONS ".:/usr/local/lib:/usr/lib"
#endif

#ifndef LIB_FILENAME
    #define LIB_FILENAME "libfakehostname." LIB_SUFFIX
#endif

#ifndef FAKE_HOSTNAME_VERSION
    #define FAKE_HOSTNAME_VERSION "unknown"
#endif


#ifdef ENABLE_DEBUG
    #include <libgen.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>

    #define COLOUR_RED   "\x1B[31m"
    #define COLOUR_RESET "\x1B[0m"

    static char _debug = 0;
    static char _isatty = -1;

    #define SET_DEBUG(n) _debug = n;
    #define DEBUG_NOFILENAME(...) \
        if (_debug) { \
            if (_isatty == -1) _isatty = isatty(fileno(stdout)); \
            if (_isatty) fputs(/* RED */ "\x1B[31m", stderr); \
            fprintf(stderr, __VA_ARGS__); \
            if (_isatty) fputs(/* RESET */ "\x1B[0m", stderr); \
        }
    #define DEBUG(...) \
        DEBUG_NOFILENAME("%s:%d: ", basename(__FILE__), __LINE__); \
        DEBUG_NOFILENAME(__VA_ARGS__);

    #define IS_DEBUG_ON() _debug

    #define SET_DEBUG_FROM_ENV() \
        SET_DEBUG(getenv(ENV_VARNAME_ENABLE_DEBUG) != NULL) \
        DEBUG("\"" ENV_VARNAME_ENABLE_DEBUG "\" set. Debug mode enabled.\n")

#else
    #define DEBUG(...)
    #define DEBUG_NOFILENAME(...)
    #define SET_DEBUG(n)
    #define SET_DEBUG_FROM_ENV()
    #define IS_DEBUG_ON() 0
#endif

#endif
