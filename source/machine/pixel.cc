
#include "pixel.h"

#include "object.h"

namespace pixel {

thread_local std::unique_ptr<Application> tApplication;

void SetApplicationForThread(std::unique_ptr<Application> application) {
  tApplication.reset();
  tApplication = std::move(application);
}

Application* ApplicationGetMain() {
  return tApplication.get();
}

Result ApplicationSetScene(Application* application, Scene* scene) {
  return Result::kFailure;
}

Scene* SceneCreate() {
  return Object<Scene>::New()->GetFFIObject();
}

void SceneCollect(Scene* scene) {
  ToObject(scene)->Release();
}

}  // namespace pixel
