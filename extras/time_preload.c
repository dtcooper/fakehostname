#include <time.h>

time_t time(__attribute__ ((unused)) time_t *seconds) {
    return (time_t) 1234567890;
}
