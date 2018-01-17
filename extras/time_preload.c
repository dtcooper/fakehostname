#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/common.h"


#ifdef ENABLE_DEBUG
    static void __attribute__((constructor)) initializer()  {
        SET_DEBUG_FROM_ENV()
    }
#endif

time_t time(__attribute__ ((unused)) time_t *tloc) {
    if (tloc != NULL) {
        puts("time(): Only implemented for tloc == NULL");
        exit(EXIT_FAILURE);
    }
    DEBUG("time(): Faked to 1234567890 [02/13/2009 15:31:30 PST]\n");
    return (time_t) 1234567890;
}
