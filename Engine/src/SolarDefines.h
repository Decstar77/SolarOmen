#pragma once

// Unsigned int types.
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

// Signed int types.
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;

// Floating point types
typedef float real32;
typedef double real64;

// Boolean types
typedef int bool32;
typedef bool bool8;

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.
STATIC_ASSERT(sizeof(uint8) == 1, "Expected uint8 to be 1 byte.");
STATIC_ASSERT(sizeof(uint16) == 2, "Expected uint16 to be 2 bytes.");
STATIC_ASSERT(sizeof(uint32) == 4, "Expected uint32 to be 4 bytes.");
STATIC_ASSERT(sizeof(uint64) == 8, "Expected uint64 to be 8 bytes.");

STATIC_ASSERT(sizeof(int8) == 1, "Expected int8 to be 1 byte.");
STATIC_ASSERT(sizeof(int16) == 2, "Expected int16 to be 2 bytes.");
STATIC_ASSERT(sizeof(int32) == 4, "Expected int32 to be 4 bytes.");
STATIC_ASSERT(sizeof(int64) == 8, "Expected int64 to be 8 bytes.");

STATIC_ASSERT(sizeof(real32) == 4, "Expected real32 to be 4 bytes.");
STATIC_ASSERT(sizeof(real64) == 8, "Expected real64 to be 8 bytes.");

#define SOL_DEBUG 1
#define SOL_DEBUG_RENDERING 1

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1


#define REAL_MAX FLT_MAX
#define REAL_MIN -FLT_MAX

#define Kilobytes(val) (val * 1024LL)
#define Megabytes(val) (Kilobytes(val) * 1024LL)
#define Gigabytes(val) (Megabytes(val) * 1024LL)

#define SetABit(x) (1 << x)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if SOL_DEBUG

#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

#define Assert(expr, msg)                                            \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            ReportAssertionFailure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }

#endif

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) 
#define SOLAR_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define KPLATFORM_LINUX 1
#if defined(__ANDROID__)
#define KPLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define KPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define KPLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define KPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define KPLATFORM_IOS 1
#define KPLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define KPLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

#ifdef SOL_EXPORT
// Exports
#ifdef _MSC_VER
#define SOL_API __declspec(dllexport)
#else
#define SOL_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define SOL_API __declspec(dllimport)
#else
#define SOL_API
#endif
#endif

namespace sol
{
	inline void ReportAssertionFailure(const char* expression, const char* message, const char* file, int32 line) {};
}