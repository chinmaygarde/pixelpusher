#pragma once

#include <memory>

#include "macros.h"
#include "object.h"

namespace pixel {

class PeerSharedObject : public std::enable_shared_from_this<PeerSharedObject> {
 public:
  PeerSharedObject() = default;

  virtual ~PeerSharedObject() = default;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(PeerSharedObject);
};

template <class _CType>
class PeerObject : public PeerSharedObject {
 public:
  using PeerAutoObject = AutoObject<_CType, std::weak_ptr<PeerSharedObject>>;

  PeerObject() = default;

  virtual ~PeerObject() = default;

  const PeerAutoObject& GetPeerObject() {
    if (object_.IsValid()) {
      return object_;
    }
    object_ = decltype(object_)::Create(this->weak_from_this());
    return object_;
  }

 private:
  PeerAutoObject object_;

  P_DISALLOW_COPY_AND_ASSIGN(PeerObject);
};

}  // namespace pixel
