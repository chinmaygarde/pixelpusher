#pragma once

#include "file.h"
#include "macros.h"

namespace pixel {

class Mapping {
 public:
  Mapping() = default;

  virtual ~Mapping() = default;

  virtual const uint8_t* GetData() const = 0;

  virtual size_t GetSize() const = 0;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(Mapping);
};

class MemoryMapping final : public Mapping {
 public:
  MemoryMapping(UniqueMapping mapping) : mapping_(std::move(mapping)) {}

  const uint8_t* GetData() const override {
    return static_cast<const uint8_t*>(mapping_.Get().mapping);
  }

  size_t GetSize() const override { return mapping_.Get().size; }

 private:
  UniqueMapping mapping_;

  P_DISALLOW_COPY_AND_ASSIGN(MemoryMapping);
};

}  // namespace pixel
