#ifndef HOMIO_CONFIG_H_
#define HOMIO_CONFIG_H_

#if __has_include (<homio.config.h>)
#include <homio.config.h>
#endif

// #define HOMIO_DEBUG

#ifdef HOMIO_DEBUG
    #define HOMIO_DEBUG(...) Serial.print(__VA_ARGS__);
    #define HOMIO_DEBUG_LN(...) Serial.println(__VA_ARGS__);
#else
    #define HOMIO_DEBUG(...)
    #define HOMIO_DEBUG_LN(...)
#endif // HOMIO_DEBUG

#endif