#ifndef UNI_SOCKETIO_H
#define UNI_SOCKETIO_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

using sockd_t = SOCKET;
using tcb_t = void;
const sockd_t NULL_SOCKD(INVALID_SOCKET);

#ifdef _WIN64
// define something for Windows (64-bit only)
#else
// define something for Windows (32-bit only)
#endif

#elif __APPLE__
#error "Unsupported platform: macOS"
//#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif

#elif defined(__linux__) || defined(__unix__)

#include <netinet/tcp.h>
using sockd_t = int;
using tcb_t = tcp_info;
constexpr sockd_t NULL_SOCKD(-1);

#else
#error "Unknown compiler"
#endif

#endif