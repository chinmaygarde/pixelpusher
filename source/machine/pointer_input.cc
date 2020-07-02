#include "pointer_input.h"

#include <algorithm>

#include "logging.h"

namespace pixel {

PointerInputDispatcher::PointerInputDispatcher() = default;

PointerInputDispatcher::~PointerInputDispatcher() = default;

bool PointerInputDispatcher::AddDelegate(PointerInputDelegate* delegate) {
  if (!delegate) {
    return false;
  }

  if (std::find(delegates_.begin(), delegates_.end(), delegate) !=
      delegates_.end()) {
    return false;
  }

  delegates_.push_back(delegate);

  return true;
}

bool PointerInputDispatcher::RemoveDelegate(PointerInputDelegate* delegate) {
  if (!delegate) {
    return false;
  }

  auto found = std::find(delegates_.begin(), delegates_.end(), delegate);
  if (found == delegates_.end()) {
    return false;
  }

  delegates_.erase(found);
  return true;
}

void PointerInputDispatcher::StopTrackingAllPointers() {
  tracked_pointers_.clear();
}

void PointerInputDispatcher::StartTrackingPointer(int64_t pointer_id) {
  tracked_pointers_[pointer_id] = std::nullopt;
}

void PointerInputDispatcher::StopTrackingPointer(int64_t pointer_id) {
  tracked_pointers_.erase(pointer_id);
}

bool PointerInputDispatcher::DispatchPointerEvent(int64_t pointer_id,
                                                  Point point) {
  auto found = tracked_pointers_.find(pointer_id);
  if (found == tracked_pointers_.end()) {
    P_ERROR << "Attempted to dispatch event to untracked pointer ID: "
            << pointer_id;
    return false;
  }

  if (!found->second.has_value()) {
    found->second = point;
  }

  const Offset offset(point.x - found->second.value().x,
                      point.y - found->second.value().y);

  for (auto i = delegates_.rbegin(); i != delegates_.rend(); i++) {
    if (!(*i)->WantsPointerInput()) {
      continue;
    }

    if ((*i)->OnPointerEvent(pointer_id, point, offset)) {
      return true;
    }
  }

  return false;
}

}  // namespace pixel
