#pragma once

#include <iostream>
#include <algorithm>
#include <assert.h>

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Log.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/SmartPointer.h"

#include "Hydra/Core/Platform.h"

#if defined(__GNUC__) || defined(__clang__) || (defined(_MSC_VER) && _MSC_VER >= 1600)
#include <stdint.h>
#elif defined(_MSC_VER)
typedef signed   __int8  int8_t;
typedef unsigned __int8  uint8_t;
typedef signed   __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed   __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed   __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
typedef int16_t wchar_t;
#else
// TODO: Have a better "if all else fails" condition.
//
// Preferably, make all sizes at least big enough for the data size.
// Also, try including C++11 standard types if possible.
//
// List of C++'s standard data sizing rules:
// 
// sizeof(char) == 1
// sizeof(char) <= sizeof(short)
// sizeof(short) <= sizeof(int)
// sizeof(int) <= sizeof(long)
// sizeof(long) <= sizeof(long long)
// sizeof(char) * CHAR_BIT >= 8
// sizeof(short) * CHAR_BIT >= 16
// sizeof(int) * CHAR_BIT >= 16
// sizeof(long) * CHAR_BIT >= 32
// sizeof(long long) * CHAR_BIT >= 64

typedef signed   char      int8_t;
typedef unsigned char      uint8_t;
typedef signed   short int int16_t;
typedef unsigned short int uint16_t;
typedef signed   int       int32_t;
typedef unsigned int       uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
typedef int16_t wchar_t;
#endif

typedef uint8_t CHART;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

#ifdef DEBUG
#define assertCheck assert
#else
#define assertCheck (void)
#endif

#ifndef FORCEINLINE
#ifdef COMPILER_MSVC
#define FORCEINLINE __forceinline
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define FORCEINLINE inline __attribute__ ((always_inline))
#else
#define FORCEINLINE inline
#endif
#endif

#if __cplusplus < 201103L
#define nullptr NULL
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

#define LOG(message) std::cout << message << std::endl;