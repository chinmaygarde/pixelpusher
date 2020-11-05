#pragma once

#include <memory>

#include "macros.h"
#include "peer_object.h"
#include "pixel_c_bindings.h"

namespace pixel {

class Application : public PeerObject<CApplication> {
 public:
  Application();

  ~Application();

 private:
  P_DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace pixel
