#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>

#include "closure.h"
#include "unshared_weak.h"
#include "vulkan.h"

namespace pixel {

class ShaderModule {
 public:
  static std::unique_ptr<ShaderModule> Load(vk::Device device,
                                            const char* shader_name,
                                            const char* debug_name);

  ~ShaderModule();

  vk::ShaderModule GetShaderModule() const;

  size_t AddLiveUpdateCallback(
      std::function<void(const ShaderModule*)> closure);

  bool RemoveLiveUpdateCallback(size_t handle);

 private:
  const vk::Device device_;
  vk::UniqueShaderModule module_;
  const std::filesystem::path original_file_path_;
  const std::string debug_name_;
  std::optional<size_t> fs_watcher_handler_;
  std::chrono::high_resolution_clock::time_point last_shader_update_ = {};
  IdentifiableCallbacks<void(const ShaderModule*)> live_update_callbacks_;
  UnsharedWeakFactory<ShaderModule> weak_factory_;

  ShaderModule(vk::Device device,
               vk::UniqueShaderModule module,
               std::filesystem::path original_file_name,
               std::string debug_name);

  void OnShaderFileDidUpdate();

  void OnShaderModuleDidUpdate(vk::UniqueShaderModule module);

  P_DISALLOW_COPY_AND_ASSIGN(ShaderModule);
};

}  // namespace pixel
