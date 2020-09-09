#pragma once

#include <string>

#include "macros.h"
#include "unique_object.h"

namespace pixel {

struct Engine;

struct EngineTraits {
  static bool IsValid(Engine* engine);

  static Engine* DefaultValue();

  static void Collect(Engine* engine);
};

class Runtime {
 public:
  Runtime(const std::string& assets_path);

  ~Runtime();

  bool IsValid() const;

 private:
  UniqueObject<Engine*, EngineTraits> engine_;

  P_DISALLOW_COPY_AND_ASSIGN(Runtime);
};

}  // namespace pixel
