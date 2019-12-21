#pragma once

#include <iostream>

#include "macros.h"

namespace pixel {

template <class Type>
class AutoLogger {
 public:
  AutoLogger(Type& logger) : logger_(logger) {}

  ~AutoLogger() {
    logger_ << std::endl;
    logger_.flush();
  }

  template <class T>
  AutoLogger& operator<<(const T& object) {
    logger_ << object;
    return *this;
  }

 private:
  Type& logger_;

  P_DISALLOW_COPY_AND_ASSIGN(AutoLogger);
};

using AutoOStreamLogger = AutoLogger<std::ostream>;

#define P_ERROR ::pixel::AutoOStreamLogger(std::cerr)
#define P_LOG ::pixel::AutoOStreamLogger(std::cout)

}  // namespace pixel
