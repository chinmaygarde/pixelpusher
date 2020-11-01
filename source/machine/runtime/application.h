#pragma once

#include <memory>

#include "macros.h"
#include "object.h"
#include "pixel_c_bindings.h"

namespace pixel {

class Application : public std::enable_shared_from_this<Application> {
 public:
  using Object = AutoObject<CApplication, std::weak_ptr<Application>>;

  static std::shared_ptr<Application> Create();

  ~Application();

  Object GetApplication() const;

 private:
  Object application_;

  Application();

  P_DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace pixel
