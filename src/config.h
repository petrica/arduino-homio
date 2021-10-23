#ifndef _HomioConfig_h_
#define _HomioConfig_h_

#if __has_include (<HomioConfig.h>)
#include <HomioConfig.h>
#endif

#include <Arduino.h>

#define HOMIO_COMMAND_HEADER_SIZE 4
#define HOMIO_BUFFER_SIZE 32
#define HOMIO_DATAPOINT_HEADER_SIZE 2
#define HOMIO_COMMAND_POOL_SIZE 3


#endif