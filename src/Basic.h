#ifndef BASIC_H
#define BASIC_H

#ifdef WIN32
#define DEBUG_BREAK() __debugbreak()
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK()
#endif

#define ASSERT(condition) do { if (!(condition)) { DEBUG_BREAK(); } } while(0)

#endif
