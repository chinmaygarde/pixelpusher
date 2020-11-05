#pragma once

#include "macros.h"
#include "object.h"
#include "pixel_c_bindings.h"
#include "runtime.h"

namespace pixel {

class Scene : public std::enable_shared_from_this<Scene> {
 public:
  using Object = AutoObject<CScene, std::weak_ptr<Scene>>;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(Scene);
};

}  // namespace pixel
