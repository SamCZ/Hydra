#pragma once

// Detect operating system
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN64)
#define OPERATING_SYSTEM_WINDOWS
#elif defined(__linux__)
#define OPERATING_SYSTEM_LINUX
#elif defined(__apple__)
#define OPERATING_SYSTEM_APPLE
// Probably detect MAC, iOS and Android at some point
#else
#define OPERATING_SYSTEM_OTHER
#endif
