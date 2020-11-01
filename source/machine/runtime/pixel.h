#pragma once

#include <memory>

#include "application.h"
#include "pixel_c_bindings.h"

namespace pixel {

void SetApplicationForThread(Application::Object application);

}  // namespace pixel
