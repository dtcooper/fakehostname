#ifndef _FAKEHOSTNAME_HAS_INCLUDED_COMMON_H
    #define _FAKEHOSTNAME_HAS_INCLUDED_COMMON_H

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
    #ifndef CFILENAME
        #error "Can't include common.h without CFILENAME defined"
    #endif

    static char _debug = 0;
    #define SET_DEBUG(n) _debug = n;
    #define DEBUG(...) if (_debug) fprintf(stderr, CFILENAME ": " __VA_ARGS__);
#else
    #define DEBUG(...)
    #define SET_DEBUG(n)
#endif

#endif
