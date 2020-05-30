#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>

#include "unshared_weak.h"
#include "vulkan.h"

namespace pixel {

class ShaderModule {
 public:
  static std::unique_ptr<ShaderModule> Load(vk::Device device,
                                            const char* shader_name);

  ~ShaderModule();

  vk::ShaderModule GetShaderModule() const;

 private:
  const vk::Device device_;
  vk::UniqueShaderModule module_;
  std::filesystem::path original_file_path_;
  std::optional<size_t> fs_watcher_handler_;
  std::chrono::high_resolution_clock::time_point last_shader_update_ = {};
  UnsharedWeakFactory<ShaderModule> weak_factory_;

  ShaderModule(vk::Device device,
               vk::UniqueShaderModule module,
               std::filesystem::path original_file_name);

  void OnShaderFileDidUpdate();

  void OnShaderModuleDidUpdate(vk::UniqueShaderModule module);

  P_DISALLOW_COPY_AND_ASSIGN(ShaderModule);
};  // namespace pixel

}  // namespace pixel
