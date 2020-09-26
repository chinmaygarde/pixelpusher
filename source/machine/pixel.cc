
#include "pixel.h"

#include "object.h"

namespace pixel {

Scene* SceneCreate() {
  return Object<Scene>::New()->GetFFIObject();
}

void SceneCollect(Scene* scene) {
  ToObject(scene)->Release();
}

}  // namespace pixel
