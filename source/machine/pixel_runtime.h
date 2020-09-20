#pragma once

#include "macros.h"
#include "runtime.h"

namespace pixel {

class PixelRuntime final : public RuntimeData {
 public:
  PixelRuntime();

  // |RuntimeData|
  ~PixelRuntime() override;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(PixelRuntime);
};

}  // namespace pixel
