#pragma once

#include <chrono>
#include <map>

#include "glm.h"
#include "macros.h"

namespace pixel {

class MatrixSimulation {
 public:
  // TODO: Make this private.
  struct UpdateValue {
    std::chrono::high_resolution_clock::time_point start_time;
    bool positive;
    double rate;
  };

  MatrixSimulation();

  ~MatrixSimulation();

  void SetInitialValue(glm::mat4 value);

  glm::mat4 GetCurrentMatrix() const;

  enum class UpdateType {
    kUnknown,
    kTranslationX,
    kTranslationY,
    kTranslationZ,
  };

  void SetUpdateRate(double rate, UpdateType type);

  void SetUpdateRate(double rate);

  void UpdateSimulation(std::chrono::high_resolution_clock::time_point now);

  void EnableUpdates(UpdateType type,
                     std::chrono::high_resolution_clock::time_point now,
                     bool positive);

  void DisableUpdates(UpdateType type);

  void Reset();

 private:
  glm::mat4 initial_ = glm::identity<glm::mat4>();
  glm::vec3 translation_ = glm::vec3{0.0};
  std::map<UpdateType, UpdateValue> updates_;
  std::map<UpdateType, double> update_rates_;

  P_DISALLOW_COPY_AND_ASSIGN(MatrixSimulation);
};

}  // namespace pixel
