#ifndef CORE_UTIL_H
#define CORE_UTIL_H

#include "Basic.h"

namespace Util {
    u64 RandomID();
    char* ReadEntireFile(const char* filename);
}

#endif