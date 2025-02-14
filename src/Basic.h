#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>
#include <stddef.h>

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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;

typedef float f32;
typedef double f64;

#endif
