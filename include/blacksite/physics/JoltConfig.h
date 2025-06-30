#pragma once

// Disable profiling to avoid linking issues
// #define JPH_PROFILE_ENABLED
// #define JPH_DEBUG_RENDERER

// Platform detection
#ifdef _WIN32
    #define JPH_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define JPH_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define JPH_PLATFORM_MACOS
#endif

// Compiler detection
#ifdef _MSC_VER
    #define JPH_COMPILER_MSVC
#elif defined(__clang__)
    #define JPH_COMPILER_CLANG
#elif defined(__GNUC__)
    #define JPH_COMPILER_GCC
#endif

// CPU architecture
#if defined(_M_X64) || defined(__x86_64__)
    #define JPH_CPU_X86_64
#elif defined(_M_IX86) || defined(__i386__)
    #define JPH_CPU_X86
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define JPH_CPU_ARM64
#elif defined(_M_ARM) || defined(__arm__)
    #define JPH_CPU_ARM
#endif

// Include the main Jolt header
#include <Jolt/Jolt.h>

// Suppress warnings for Jolt includes
JPH_SUPPRESS_WARNINGS