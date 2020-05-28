#include "mapping.h"

namespace pixel {

Mapping::Mapping() = default;

Mapping::~Mapping() = default;

class DataMapping : public Mapping {
 public:
  DataMapping(const uint8_t* data, size_t size, Closure deleter)
      : data_(data), size_(size), deleter_(deleter) {}

  // |Mapping|
  ~DataMapping() override {
    if (deleter_) {
      deleter_();
    }
  }

  // |Mapping|
  const uint8_t* GetData() const override { return data_; }

  // |Mapping|s
  size_t GetSize() const override { return size_; }

 private:
  const uint8_t* data_ = nullptr;
  size_t size_ = 0;
  Closure deleter_ = nullptr;

  P_DISALLOW_COPY_AND_ASSIGN(DataMapping);
};

std::unique_ptr<Mapping> CopyMapping(const uint8_t* data, size_t size) {
  auto allocation = ::malloc(size);
  if (!allocation) {
    return nullptr;
  }
  ::memcpy(allocation, data, size);
  auto deleter = [allocation]() { ::free(allocation); };

  return std::make_unique<DataMapping>(
      reinterpret_cast<const uint8_t*>(allocation), size, deleter);
}

std::unique_ptr<Mapping> UnownedMapping(const uint8_t* data,
                                        size_t size,
                                        Closure closure) {
  return std::make_unique<DataMapping>(data, size, closure);
}

}  // namespace pixel
