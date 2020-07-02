#pragma once

#include <map>
#include <optional>
#include <vector>

#include "geometry.h"
#include "macros.h"

namespace pixel {

class PointerInputDelegate {
 public:
  virtual bool WantsPointerInput() = 0;

  virtual bool OnPointerEvent(int64_t pointer_id,
                              Point point,
                              Offset offset) = 0;
};

class PointerInputDispatcher {
 public:
  PointerInputDispatcher();

  ~PointerInputDispatcher();

  bool AddDelegate(PointerInputDelegate* delegate);

  bool RemoveDelegate(PointerInputDelegate* delegate);

  void StopTrackingAllPointers();

  void StartTrackingPointer(int64_t pointer_id);

  void StopTrackingPointer(int64_t pointer_id);

  bool DispatchPointerEvent(int64_t pointer_id, Point point);

 private:
  std::vector<PointerInputDelegate*> delegates_;
  std::map<int64_t, std::optional<Point>> tracked_pointers_;

  P_DISALLOW_COPY_AND_ASSIGN(PointerInputDispatcher);
};

}  // namespace pixel
