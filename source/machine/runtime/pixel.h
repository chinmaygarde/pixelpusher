#pragma once

#include <memory>

#include "pixel_c_bindings.h"

namespace pixel {

class Application;

void SetApplicationForThread(std::shared_ptr<Application> application);

}  // namespace pixel
