#pragma once

#include <memory>
#include <string>
#include <vector>

#include "file.h"
#include "macros.h"
#include "unique_object.h"

namespace pixel {

struct Engine;

struct EngineTraits {
  static bool IsValid(Engine* engine);

  static Engine* DefaultValue();

  static void Collect(Engine* engine);
};

class RuntimeArgs {
 public:
  RuntimeArgs();

  ~RuntimeArgs();

  void SetAssetsPath(std::string assets_path = GetCurrentExecutablePath()
                                                   .make_preferred()
                                                   .remove_filename()
                                                   .string());

  const std::string& GetAssetsPath() const;

  void SetCommandLineArgs(int argc, char const* argv[]);

  void AddCommandLineArg(std::string arg);

  const std::vector<std::string>& GetCommandLineArgs() const;

 private:
  std::string assets_path_;
  std::vector<std::string> command_line_args_;

  P_DISALLOW_COPY_AND_ASSIGN(RuntimeArgs);
};

class RuntimeData {
 public:
  RuntimeData();

  virtual ~RuntimeData();
};

class Runtime {
 public:
  static void AttachToCurrentThread(std::shared_ptr<Runtime> runtime);

  static void ClearCurrentThreadRuntime();

  static Runtime* GetCurrentRuntime();

  static RuntimeData* GetCurrentRuntimeData();

  Runtime(const RuntimeArgs& args, std::unique_ptr<RuntimeData> data);

  ~Runtime();

  bool IsValid() const;

  RuntimeData* GetRuntimeData() const;

 private:
  UniqueObject<Engine*, EngineTraits> engine_;
  std::unique_ptr<RuntimeData> data_;

  P_DISALLOW_COPY_AND_ASSIGN(Runtime);
};

}  // namespace pixel
