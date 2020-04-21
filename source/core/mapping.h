#pragma once

#include "file.h"
#include "macros.h"

namespace pixel {

class Mapping {
 public:
  Mapping() = default;

  virtual ~Mapping() = default;

  virtual const uint8_t* GetData() const = 0;

  virtual size_t GetSize() const = 0;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(Mapping);
};

}  // namespace pixel
