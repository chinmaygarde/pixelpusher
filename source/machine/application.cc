#include "application.h"

#include "pixel.h"

namespace pixel {

std::shared_ptr<Application> Application::Create() {
  return std::shared_ptr<Application>(new Application());
}

Application::Application() : application_(Object::Create(weak_from_this())) {}

Application::~Application() = default;

Application::Object Application::GetApplication() const {
  return application_;
}

}  // namespace pixel
