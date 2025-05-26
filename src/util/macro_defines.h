#ifndef UTIL_MACRO_DEFINES_H_
#define UTIL_MACRO_DEFINES_H_
#include <stddef.h>

#ifdef WEBRTC_WIN
#ifdef BUILD_COMMON_SHARED_DLL
#define COMMON_DLLEXPORT __declspec(dllexport)
#else
#define COMMON_DLLEXPORT __declspec(dllimport)
#endif

#elif defined(WEBRTC_MAC) || defined (WEBRTC_IOS) || defined (WEBRTC_ANDROID) || defined (WEBRTC_HOS)
#define COMMON_DLLEXPORT __attribute__((visibility("default")))
#else
#define COMMON_DLLEXPORT
#endif

#ifndef NULL
#define NULL 0
#endif

#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
#define WEBRTC_MAC_ONLY
#endif

#endif // UTIL_MACRO_DEFINES_H_
