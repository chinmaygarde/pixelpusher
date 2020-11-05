#include "pixel.h"

#include "application.h"
#include "object.h"

namespace pixel {

thread_local std::shared_ptr<Application> tApplication;

////////////////////////////////////////////////////////////////////////////////
/// Application
////////////////////////////////////////////////////////////////////////////////

void SetApplicationForThread(std::shared_ptr<Application> application) {
  tApplication = std::move(application);
}

CApplication* ApplicationGetMain() {
  if (!tApplication) {
    return nullptr;
  }

  return tApplication->GetPeerObject().Get()->Get();
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
