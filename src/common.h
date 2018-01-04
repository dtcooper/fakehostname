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
#ifndef ENV_VARNAME_ENABLE_VERBOSE
    #define ENV_VARNAME_ENABLE_VERBOSE "FAKE_HOSTNAME_ENABLE_VERBOSE"
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
