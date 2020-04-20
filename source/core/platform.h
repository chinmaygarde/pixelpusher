#pragma once

/*
 *  Platform
 */

#if __APPLE__

#define P_OS_MAC 1

#elif __linux__

#define P_OS_LINUX 1

#if __ANDROID__
/*
 *  Android is a Linux subtype
 */
#define P_OS_ANDROID 1

#endif

#elif __native_client__

#define P_OS_NACL 1

#elif _WIN32

#define P_OS_WINDOWS 1
#define P_OS_WIN 1

#elif __FreeBSD__

#define P_OS_BSD 1

#elif __fuchsia__

#define P_OS_FUCHSIA 1

#else

#error Unsupported Platform.

#endif
