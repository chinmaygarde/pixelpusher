#pragma once

#include <errno.h>

#include <cassert>

#define P_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;        \
  TypeName& operator=(const TypeName&) = delete;

#if defined(__GNUC__) || defined(__clang__)
#define P_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define P_WARN_UNUSED_RESULT
#endif

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
