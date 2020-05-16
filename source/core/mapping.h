#pragma once

#include <memory>

#include "macros.h"

namespace pixel {

class Mapping {
 public:
  Mapping();

  virtual ~Mapping();

  virtual const uint8_t* GetData() const = 0;

  virtual size_t GetSize() const = 0;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(Mapping);
};

std::unique_ptr<Mapping> CopyMapping(const uint8_t* data, size_t size);

}  // namespace pixel
