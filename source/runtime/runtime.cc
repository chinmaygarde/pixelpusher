#include "runtime.h"

#include "flutter_embedder.h"
#include "logging.h"

namespace pixel {

Runtime::Runtime(const std::string& assets_path) {
  FlutterProjectArgs project_args = {};
  project_args.struct_size = sizeof(project_args);
  project_args.assets_path = assets_path.c_str();

  FlutterSoftwareRendererConfig software_config = {};
  software_config.struct_size = sizeof(software_config);
  software_config.surface_present_callback =
      [](void* user_data, const void* allocation, size_t row_bytes,
         size_t height) -> bool {
    P_ERROR << "Unsupported engine tried to render a frame.";
    return true;
  };

  FlutterRendererConfig renderer_config = {};
  renderer_config.type = FlutterRendererType::kSoftware;
  renderer_config.software = software_config;

  FlutterEngine engine = nullptr;
  if (FlutterEngineRun(FLUTTER_ENGINE_VERSION, &renderer_config, &project_args,
                       this, &engine) != kSuccess) {
    P_ERROR << "Could not launch the runtime.";
    return;
  }

  engine_.Reset(reinterpret_cast<Engine*>(engine));
}

Runtime::~Runtime() = default;

bool Runtime::IsValid() const {
  return engine_.IsValid();
}

bool EngineTraits::IsValid(Engine* engine) {
  return engine != nullptr;
}

Engine* EngineTraits::DefaultValue() {
  return nullptr;
}

void EngineTraits::Collect(Engine* engine) {
  auto flutter_engine = reinterpret_cast<FlutterEngine>(engine);
  if (FlutterEngineShutdown(flutter_engine) != kSuccess) {
    P_ERROR << "Could not shutdown the Flutter Engine.";
  }
}

}  // namespace pixel
