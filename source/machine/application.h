#pragma once

#include "macros.h"
#include "object.h"
#include "pixel.h"

namespace pixel {

class ApplicationPeer : public Object<Application> {
 public:
  ApplicationPeer();

  ~ApplicationPeer();

 private:
  P_DISALLOW_COPY_AND_ASSIGN(ApplicationPeer);
};

}  // namespace pixel
