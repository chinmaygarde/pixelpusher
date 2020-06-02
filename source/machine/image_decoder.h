#pragma once

#include <memory>

#include "command_pool.h"
#include "geometry.h"
#include "image.h"
#include "macros.h"
#include "mapping.h"
#include "memory_allocator.h"
#include "vulkan.h"

namespace pixel {

class ImageDecoder {
 public:
  ImageDecoder(const Mapping& mapping);

  ~ImageDecoder();

  bool IsValid() const;

  Size GetSize() const;

  std::unique_ptr<ImageView> CreateDeviceLocalImageCopy(
      MemoryAllocator& allocator,
      const CommandPool& pool,
      const char* debug_name,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done) const;

 private:
  std::shared_ptr<void> image_;
  int stb_format_ = 0;
  size_t image_data_size_ = 0;
  Size size_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(ImageDecoder);
};

}  // namespace pixel
