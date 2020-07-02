#pragma once

#include <map>
#include <optional>
#include <vector>

#include "geometry.h"
#include "macros.h"

namespace pixel {

enum class PointerPhase {
  kPointerPhaseBegin,
  kPointerPhaseMove,
  kPointerPhaseEnd,
  kPointerPhaseCancel,
};

struct PointerData {
  Point start_point;
  Point last_point;

  PointerData(Point start) : start_point(start), last_point(start) {}

  constexpr Offset GetOffset() const { return last_point - start_point; }
};

class PointerInputDelegate {
 public:
  virtual bool WantsPointerInput() = 0;

  virtual bool OnPointerEvent(int64_t pointer_id,
                              PointerPhase phase,
                              const PointerData& data) = 0;
};

class PointerInputDispatcher {
 public:
  PointerInputDispatcher();

  ~PointerInputDispatcher();

  bool AddDelegate(PointerInputDelegate* delegate);

  bool RemoveDelegate(PointerInputDelegate* delegate);

  void StartTrackingPointer(int64_t pointer_id);

  void StopTrackingPointer(int64_t pointer_id, bool cancel);

  bool DispatchPointerEvent(int64_t pointer_id, Point point);

 private:
  std::vector<PointerInputDelegate*> delegates_;
  std::map<int64_t, std::optional<PointerData>> tracked_pointers_;

  bool DispatchPointerEvent(int64_t pointer_id,
                            PointerPhase phase,
                            const PointerData& data) const;

  P_DISALLOW_COPY_AND_ASSIGN(PointerInputDispatcher);
};

}  // namespace pixel
