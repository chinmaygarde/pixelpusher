#pragma once

#include <memory>

#include "pixel_c_bindings.h"
#include "pixel_runtime.h"
#include "renderer.h"

namespace pixel {

void SetApplicationForThread(std::unique_ptr<Application> application);

}  // namespace pixel
