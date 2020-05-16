#include "mapping.h"

namespace pixel {

Mapping::Mapping() = default;

Mapping::~Mapping() = default;

class DataMapping : public Mapping {
 public:
  DataMapping(const uint8_t* data, size_t size) {
    if (size == 0) {
      return;
    }
    auto allocation = ::malloc(size);
    if (allocation == nullptr) {
      return;
    }
    ::memmove(allocation, data, size);
    data_ = reinterpret_cast<uint8_t*>(allocation);
    size_ = size;
  }

  // |Mapping|
  ~DataMapping() override { ::free(data_); }

  // |Mapping|
  const uint8_t* GetData() const override { return data_; }

  // |Mapping|s
  size_t GetSize() const override { return size_; }

 private:
  uint8_t* data_ = nullptr;
  size_t size_ = 0;

  P_DISALLOW_COPY_AND_ASSIGN(DataMapping);
};

std::unique_ptr<Mapping> CopyMapping(const uint8_t* data, size_t size) {
  auto mapping = std::make_unique<DataMapping>(data, size);
  if (mapping->GetSize() != size) {
    return nullptr;
  }
  return mapping;
}

}  // namespace pixel
