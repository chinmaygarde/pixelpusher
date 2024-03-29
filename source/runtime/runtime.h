#pragma once

#include <memory>
#include <string>
#include <vector>

#include "closure.h"
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

  void AddRootIsolateCreateCallback(Closure callback);

  const std::vector<std::string>& GetCommandLineArgs() const;

  std::vector<Closure> GetRootIsolateCreateCallbacks() const;

 private:
  std::string assets_path_;
  std::vector<std::string> command_line_args_;
  std::vector<Closure> root_isolate_create_callbacks_;

  P_DISALLOW_COPY_AND_ASSIGN(RuntimeArgs);
};

class Runtime {
 public:
  Runtime(const RuntimeArgs& args);

  ~Runtime();

  bool IsValid() const;

 private:
  UniqueObject<Engine*, EngineTraits> engine_;
  std::vector<Closure> root_isolate_create_callbacks_;

  void DidCreateRootIsolate() const;

  P_DISALLOW_COPY_AND_ASSIGN(Runtime);
};

}  // namespace pixel
