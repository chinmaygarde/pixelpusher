#include "runtime.h"

#include "flutter_embedder.h"
#include "logging.h"

namespace pixel {

RuntimeArgs::RuntimeArgs() = default;

RuntimeArgs::~RuntimeArgs() = default;

void RuntimeArgs::SetAssetsPath(std::string assets_path) {
  assets_path_ = std::move(assets_path);
}

const std::string& RuntimeArgs::GetAssetsPath() const {
  return assets_path_;
}

void RuntimeArgs::SetCommandLineArgs(int argc, char const* argv[]) {
  for (int i = 0; i < argc; i++) {
    command_line_args_.push_back(argv[i]);
  }
}

void RuntimeArgs::AddCommandLineArg(std::string arg) {
  command_line_args_.emplace_back(std::move(arg));
}

const std::vector<std::string>& RuntimeArgs::GetCommandLineArgs() const {
  return command_line_args_;
}

RuntimeData::RuntimeData() = default;

RuntimeData::~RuntimeData() = default;

Runtime::Runtime(const RuntimeArgs& args, std::unique_ptr<RuntimeData> data)
    : data_(std::move(data)) {
  FlutterProjectArgs project_args = {};
  project_args.struct_size = sizeof(project_args);
  project_args.assets_path = args.GetAssetsPath().c_str();

  auto command_line_args = args.GetCommandLineArgs();
  std::vector<const char*> c_command_line_args;
  for (const auto& arg : command_line_args) {
    c_command_line_args.push_back(arg.c_str());
  }
  project_args.command_line_argc = c_command_line_args.size();
  project_args.command_line_argv = c_command_line_args.data();

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

RuntimeData* Runtime::GetRuntimeData() const {
  return data_.get();
}

thread_local std::shared_ptr<Runtime> tRuntime;

void Runtime::AttachToCurrentThread(std::shared_ptr<Runtime> runtime) {
  tRuntime = std::move(runtime);
}

void Runtime::ClearCurrentThreadRuntime() {
  tRuntime.reset();
}

Runtime* Runtime::GetCurrentRuntime() {
  P_ASSERT(static_cast<bool>(tRuntime) &&
           "Thread must have a current runtime associated with it.");
  return tRuntime.get();
}

RuntimeData* Runtime::GetCurrentRuntimeData() {
  return GetCurrentRuntime()->GetRuntimeData();
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
