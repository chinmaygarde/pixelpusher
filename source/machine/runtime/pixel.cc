#include "pixel.h"

#include "object.h"

namespace pixel {

thread_local Application::Object tApplication;

////////////////////////////////////////////////////////////////////////////////
/// Application
////////////////////////////////////////////////////////////////////////////////

void SetApplicationForThread(Application::Object application) {
  tApplication = std::move(application);
}

CApplication* ApplicationGetMain() {
  if (!tApplication.IsValid()) {
    return nullptr;
  }
  return tApplication->Get();
}

CResult ApplicationSetScene(CApplication* application, CScene* scene) {
  if (!application || !scene) {
    return CResult::kFailure;
  }
  return CResult::kSuccess;
}

////////////////////////////////////////////////////////////////////////////////
/// Scene
////////////////////////////////////////////////////////////////////////////////

CScene* SceneCreate() {
  return AutoObject<CScene>::Create().TakeOwnership()->Get();
}

void SceneCollect(CScene* scene) {
  ToObject(scene)->Release();
}

}  // namespace pixel
