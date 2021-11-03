#ifndef _HomioConfig_h_
#define _HomioConfig_h_

#if __has_include (<HomioConfig.h>)
#include <HomioConfig.h>
#endif

#include <Arduino.h>

#ifndef HOMIO_COMMAND_HEADER_SIZE
    #define HOMIO_COMMAND_HEADER_SIZE 4
#endif
#ifndef HOMIO_COMMAND_QUEUE_SIZE
    #define HOMIO_COMMAND_QUEUE_SIZE 32
#endif
#ifndef HOMIO_BUFFER_SIZE
    #define HOMIO_BUFFER_SIZE 32
#endif
#ifndef HOMIO_DATAPOINT_HEADER_SIZE
    #define HOMIO_DATAPOINT_HEADER_SIZE 2
#endif
#ifndef HOMIO_COMMAND_POOL_SIZE
    #define HOMIO_COMMAND_POOL_SIZE 3
#endif

#endif