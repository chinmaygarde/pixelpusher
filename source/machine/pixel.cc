
#include "pixel.h"

namespace pixel {

// static PixelRuntime* GetCurrentPixelRuntime() {
//   return static_cast<PixelRuntime*>(Runtime::GetCurrentRuntimeData());
// }

Renderer* GetRenderer() {
  return nullptr;
}

Result RendererSetScene(Renderer* renderer, Scene* scene) {
  return Result::kFailure;
}

}  // namespace pixel
