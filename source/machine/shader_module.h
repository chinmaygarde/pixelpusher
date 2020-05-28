#pragma once

#include <memory>

#include "vulkan.h"

namespace pixel {

class ShaderModule {
 public:
  static std::unique_ptr<ShaderModule> Load(const vk::Device& device,
                                            const char* shader_name);

  ~ShaderModule();

  vk::ShaderModule GetShaderModule() const;

  std::string GetOriginalFileName() const;

 private:
  vk::UniqueShaderModule module_;
  std::string original_file_name_;

  ShaderModule(vk::UniqueShaderModule module, std::string original_file_name);

  P_DISALLOW_COPY_AND_ASSIGN(ShaderModule);
};

}  // namespace pixel
