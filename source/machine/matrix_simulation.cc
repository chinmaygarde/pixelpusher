#include "matrix_simulation.h"

namespace pixel {

MatrixSimulation::MatrixSimulation() {
  SetUpdateRate(1.0);
}

MatrixSimulation::~MatrixSimulation() = default;

void MatrixSimulation::SetInitialValue(glm::mat4 value) {
  initial_ = value;
}

glm::mat4 MatrixSimulation::GetCurrentMatrix() const {
  const auto translation =
      glm::translate(glm::identity<glm::mat4>(), translation_);

  auto rotation_x =
      glm::rotate(glm::identity<glm::quat>(), glm::radians<float>(rotation_x_),
                  glm::vec3(1.0, 0.0, 0.0));

  auto rotation_y =
      glm::rotate(glm::identity<glm::quat>(), glm::radians<float>(rotation_y_),
                  glm::vec3(0.0, 1.0, 0.0));

  auto rotation = glm::mat4(rotation_x * rotation_y);

  return translation * rotation * initial_;
}

template <class T>
void ApplyUpdate(T& value,
                 std::chrono::high_resolution_clock::time_point now,
                 const MatrixSimulation::UpdateValue& update) {
  using Seconds = std::chrono::duration<double, std::ratio<1>>;
  const auto duration =
      std::chrono::duration_cast<Seconds>(now - update.start_time).count();
  if (update.positive) {
    value += duration * update.rate;
  } else {
    value -= duration * update.rate;
  }
}

void MatrixSimulation::UpdateSimulation(
    std::chrono::high_resolution_clock::time_point now) {
  for (const auto& update : updates_) {
    switch (update.first) {
      case UpdateType::kTranslationX:
        ApplyUpdate(translation_.x, now, update.second);
        break;
      case UpdateType::kTranslationY:
        ApplyUpdate(translation_.y, now, update.second);
        break;
      case UpdateType::kTranslationZ:
        ApplyUpdate(translation_.z, now, update.second);
        break;
      case UpdateType::kRotationX:
        ApplyUpdate(rotation_x_, now, update.second);
        break;
      case UpdateType::kRotationY:
        ApplyUpdate(rotation_y_, now, update.second);
        break;
      case UpdateType::kUnknown:
        continue;
    }
  }
}

void MatrixSimulation::EnableUpdates(
    UpdateType type,
    std::chrono::high_resolution_clock::time_point now,
    bool positive) {
  if (type == UpdateType::kUnknown) {
    return;
  }
  updates_[type] = {now, positive, update_rates_.at(type)};
}

void MatrixSimulation::DisableUpdates(UpdateType type) {
  if (type == UpdateType::kUnknown) {
    return;
  }
  updates_.erase(type);
}

void MatrixSimulation::SetUpdateRate(double rate, UpdateType type) {
  update_rates_[type] = rate;
}

void MatrixSimulation::SetUpdateRate(double rate) {
  SetUpdateRate(rate, UpdateType::kTranslationX);
  SetUpdateRate(rate, UpdateType::kTranslationY);
  SetUpdateRate(rate, UpdateType::kTranslationZ);
  SetUpdateRate(rate, UpdateType::kRotationX);
  SetUpdateRate(rate, UpdateType::kRotationY);
}

void MatrixSimulation::Reset() {
  updates_.clear();
  translation_ = glm::vec3{0.0};
  rotation_x_ = 0.0;
  rotation_y_ = 0.0;
}

}  // namespace pixel
