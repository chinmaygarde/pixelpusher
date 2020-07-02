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

void PointerInputDispatcher::StartTrackingPointer(int64_t pointer_id) {
  auto found = tracked_pointers_.find(pointer_id);
  if (found != tracked_pointers_.end()) {
    StopTrackingPointer(pointer_id, true /* cancel */);
  }

  tracked_pointers_[pointer_id] = std::nullopt;
}

void PointerInputDispatcher::StopTrackingPointer(int64_t pointer_id,
                                                 bool cancel) {
  auto found = tracked_pointers_.find(pointer_id);
  if (found == tracked_pointers_.end()) {
    return;
  }

  auto found_data = found->second;

  tracked_pointers_.erase(found);

  if (!found_data.has_value()) {
    return;
  }

  const auto phase = cancel ? PointerPhase::kPointerPhaseCancel
                            : PointerPhase::kPointerPhaseEnd;
  DispatchPointerEvent(pointer_id, phase, found_data.value());
}

bool PointerInputDispatcher::DispatchPointerEvent(int64_t pointer_id,
                                                  Point point) {
  auto found = tracked_pointers_.find(pointer_id);
  if (found == tracked_pointers_.end()) {
    P_ERROR << "Attempted to dispatch event to untracked pointer ID: "
            << pointer_id;
    return false;
  }

  PointerPhase phase = PointerPhase::kPointerPhaseMove;

  if (!found->second.has_value()) {
    found->second = {point};
    phase = PointerPhase::kPointerPhaseBegin;
  }

  found->second.value().last_point = point;

  return DispatchPointerEvent(pointer_id, phase, found->second.value());
}

bool PointerInputDispatcher::DispatchPointerEvent(
    int64_t pointer_id,
    PointerPhase phase,
    const PointerData& data) const {
  for (auto i = delegates_.rbegin(); i != delegates_.rend(); i++) {
    if (!(*i)->WantsPointerInput()) {
      continue;
    }

    if ((*i)->OnPointerEvent(pointer_id, phase, data)) {
      return true;
    }
  }
  return false;
}

}  // namespace pixel
