#ifndef CORE_UTIL_H
#define CORE_UTIL_H

#include "Basic.h"

#include <string>

namespace Util {
    u64 RandomID();
    std::string ReadEntireFile(const std::string& filename);
}

#endif