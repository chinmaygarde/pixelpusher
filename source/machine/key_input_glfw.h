#pragma once

#include "key_input.h"
#include "macros.h"

namespace pixel {

KeyType GLFWKeyTypeToKeyType(int key_type);

KeyModifiers GLFWKeyModifiersToModifiers(int modifiers);

KeyAction GLFWKeyActionToAction(int key_action);

}  // namespace pixel
