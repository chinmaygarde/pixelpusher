#include "key_input.h"

namespace pixel {

KeyInputDispatcher::KeyInputDispatcher() = default;

KeyInputDispatcher::~KeyInputDispatcher() = default;

bool KeyInputDispatcher::AddDelegate(KeyInputDelegate* delegate) {
  if (!delegate) {
    return false;
  }

  auto found = delegates_.find(delegate);
  if (found != delegates_.end()) {
    return false;
  }

  delegates_.insert(delegate);
  return true;
}

bool KeyInputDispatcher::RemoveDelegate(KeyInputDelegate* delegate) {
  if (!delegate) {
    return false;
  }

  auto found = delegates_.find(delegate);
  if (found == delegates_.end()) {
    return false;
  }

  delegates_.erase(found);
  return true;
}

void KeyInputDispatcher::DispatchKey(KeyType type,
                                     KeyAction action,
                                     KeyModifiers modifier) {
  for (const auto& delegate : delegates_) {
    if (delegate->WantsKeyEvents()) {
      delegate->OnKeyEvent(type, action, modifier);
    }
  }
}

}  // namespace pixel
