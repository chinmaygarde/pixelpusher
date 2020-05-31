#pragma once

#include <errno.h>

#include <cassert>

#include "platform.h"

#define P_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;        \
  TypeName& operator=(const TypeName&) = delete;

#if defined(__GNUC__) || defined(__clang__)
#define CLANG_OR_GCC 1
#else  // defined(__GNUC__) || defined(__clang__)
#define CLANG_OR_GCC 0
#endif  //  defined(__GNUC__) || defined(__clang__)

#if CLANG_OR_GCC
#define P_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define P_PRINTF_FORMAT(format_number, args_number) \
  __attribute__((format(printf, format_number, args_number)))
#define GCC_PRAGMA(x) _Pragma(x)
#else  // CLANG_OR_GCC
#define P_WARN_UNUSED_RESULT
#define P_PRINTF_FORMAT(format_number, args_number)
#define GCC_PRAGMA(x)
#endif  // CLANG_OR_GCC

#define P_ASSERT(x) assert((x))

#define P_ALLOW_UNUSED_LOCAL(x) false ? (void)x : (void)0

#define P_TEMP_FAILURE_RETRY(exp)          \
  ({                                       \
    __typeof__(exp) _rc;                   \
    do {                                   \
      _rc = (exp);                         \
    } while (_rc == -1 && errno == EINTR); \
    _rc;                                   \
  })
