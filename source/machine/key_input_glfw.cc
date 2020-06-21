#include "key_input_glfw.h"

#include "glfw.h"

namespace pixel {

KeyType GLFWKeyTypeToKeyType(int key_type) {
  switch (key_type) {
    case GLFW_KEY_SPACE:
      return KeyType::kKeyTypeSpace;
    case GLFW_KEY_APOSTROPHE:
      return KeyType::kKeyTypeApostrophe;
    case GLFW_KEY_COMMA:
      return KeyType::kKeyTypeComma;
    case GLFW_KEY_MINUS:
      return KeyType::kKeyTypeMinus;
    case GLFW_KEY_PERIOD:
      return KeyType::kKeyTypePeriod;
    case GLFW_KEY_SLASH:
      return KeyType::kKeyTypeSlash;
    case GLFW_KEY_0:
      return KeyType::kKeyType0;
    case GLFW_KEY_1:
      return KeyType::kKeyType1;
    case GLFW_KEY_2:
      return KeyType::kKeyType2;
    case GLFW_KEY_3:
      return KeyType::kKeyType3;
    case GLFW_KEY_4:
      return KeyType::kKeyType4;
    case GLFW_KEY_5:
      return KeyType::kKeyType5;
    case GLFW_KEY_6:
      return KeyType::kKeyType6;
    case GLFW_KEY_7:
      return KeyType::kKeyType7;
    case GLFW_KEY_8:
      return KeyType::kKeyType8;
    case GLFW_KEY_9:
      return KeyType::kKeyType9;
    case GLFW_KEY_SEMICOLON:
      return KeyType::kKeyTypeSemicolon;
    case GLFW_KEY_EQUAL:
      return KeyType::kKeyTypeEqual;
    case GLFW_KEY_A:
      return KeyType::kKeyTypeA;
    case GLFW_KEY_B:
      return KeyType::kKeyTypeB;
    case GLFW_KEY_C:
      return KeyType::kKeyTypeC;
    case GLFW_KEY_D:
      return KeyType::kKeyTypeD;
    case GLFW_KEY_E:
      return KeyType::kKeyTypeE;
    case GLFW_KEY_F:
      return KeyType::kKeyTypeF;
    case GLFW_KEY_G:
      return KeyType::kKeyTypeG;
    case GLFW_KEY_H:
      return KeyType::kKeyTypeH;
    case GLFW_KEY_I:
      return KeyType::kKeyTypeI;
    case GLFW_KEY_J:
      return KeyType::kKeyTypeJ;
    case GLFW_KEY_K:
      return KeyType::kKeyTypeK;
    case GLFW_KEY_L:
      return KeyType::kKeyTypeL;
    case GLFW_KEY_M:
      return KeyType::kKeyTypeM;
    case GLFW_KEY_N:
      return KeyType::kKeyTypeN;
    case GLFW_KEY_O:
      return KeyType::kKeyTypeO;
    case GLFW_KEY_P:
      return KeyType::kKeyTypeP;
    case GLFW_KEY_Q:
      return KeyType::kKeyTypeQ;
    case GLFW_KEY_R:
      return KeyType::kKeyTypeR;
    case GLFW_KEY_S:
      return KeyType::kKeyTypeS;
    case GLFW_KEY_T:
      return KeyType::kKeyTypeT;
    case GLFW_KEY_U:
      return KeyType::kKeyTypeU;
    case GLFW_KEY_V:
      return KeyType::kKeyTypeV;
    case GLFW_KEY_W:
      return KeyType::kKeyTypeW;
    case GLFW_KEY_X:
      return KeyType::kKeyTypeX;
    case GLFW_KEY_Y:
      return KeyType::kKeyTypeY;
    case GLFW_KEY_Z:
      return KeyType::kKeyTypeZ;
    case GLFW_KEY_LEFT_BRACKET:
      return KeyType::kKeyTypeLeftBracket;
    case GLFW_KEY_BACKSLASH:
      return KeyType::kKeyTypeBackslash;
    case GLFW_KEY_RIGHT_BRACKET:
      return KeyType::kKeyTypeRightBracket;
    case GLFW_KEY_GRAVE_ACCENT:
      return KeyType::kKeyTypeGraveAccent;
    case GLFW_KEY_WORLD_1:
      return KeyType::kKeyTypeWorld1;
    case GLFW_KEY_WORLD_2:
      return KeyType::kKeyTypeWorld2;
    case GLFW_KEY_ESCAPE:
      return KeyType::kKeyTypeEscape;
    case GLFW_KEY_ENTER:
      return KeyType::kKeyTypeEnter;
    case GLFW_KEY_TAB:
      return KeyType::kKeyTypeTab;
    case GLFW_KEY_BACKSPACE:
      return KeyType::kKeyTypeBackspace;
    case GLFW_KEY_INSERT:
      return KeyType::kKeyTypeInsert;
    case GLFW_KEY_DELETE:
      return KeyType::kKeyTypeDelete;
    case GLFW_KEY_RIGHT:
      return KeyType::kKeyTypeRight;
    case GLFW_KEY_LEFT:
      return KeyType::kKeyTypeLeft;
    case GLFW_KEY_DOWN:
      return KeyType::kKeyTypeDown;
    case GLFW_KEY_UP:
      return KeyType::kKeyTypeUp;
    case GLFW_KEY_PAGE_UP:
      return KeyType::kKeyTypePageUp;
    case GLFW_KEY_PAGE_DOWN:
      return KeyType::kKeyTypePageDown;
    case GLFW_KEY_HOME:
      return KeyType::kKeyTypeHome;
    case GLFW_KEY_END:
      return KeyType::kKeyTypeEnd;
    case GLFW_KEY_CAPS_LOCK:
      return KeyType::kKeyTypeCapsLock;
    case GLFW_KEY_SCROLL_LOCK:
      return KeyType::kKeyTypeScrollLock;
    case GLFW_KEY_NUM_LOCK:
      return KeyType::kKeyTypeNumLock;
    case GLFW_KEY_PRINT_SCREEN:
      return KeyType::kKeyTypePrintScreen;
    case GLFW_KEY_PAUSE:
      return KeyType::kKeyTypePause;
    case GLFW_KEY_F1:
      return KeyType::kKeyTypeF1;
    case GLFW_KEY_F2:
      return KeyType::kKeyTypeF2;
    case GLFW_KEY_F3:
      return KeyType::kKeyTypeF3;
    case GLFW_KEY_F4:
      return KeyType::kKeyTypeF4;
    case GLFW_KEY_F5:
      return KeyType::kKeyTypeF5;
    case GLFW_KEY_F6:
      return KeyType::kKeyTypeF6;
    case GLFW_KEY_F7:
      return KeyType::kKeyTypeF7;
    case GLFW_KEY_F8:
      return KeyType::kKeyTypeF8;
    case GLFW_KEY_F9:
      return KeyType::kKeyTypeF9;
    case GLFW_KEY_F10:
      return KeyType::kKeyTypeF10;
    case GLFW_KEY_F11:
      return KeyType::kKeyTypeF11;
    case GLFW_KEY_F12:
      return KeyType::kKeyTypeF12;
    case GLFW_KEY_F13:
      return KeyType::kKeyTypeF13;
    case GLFW_KEY_F14:
      return KeyType::kKeyTypeF14;
    case GLFW_KEY_F15:
      return KeyType::kKeyTypeF15;
    case GLFW_KEY_F16:
      return KeyType::kKeyTypeF16;
    case GLFW_KEY_F17:
      return KeyType::kKeyTypeF17;
    case GLFW_KEY_F18:
      return KeyType::kKeyTypeF18;
    case GLFW_KEY_F19:
      return KeyType::kKeyTypeF19;
    case GLFW_KEY_F20:
      return KeyType::kKeyTypeF20;
    case GLFW_KEY_F21:
      return KeyType::kKeyTypeF21;
    case GLFW_KEY_F22:
      return KeyType::kKeyTypeF22;
    case GLFW_KEY_F23:
      return KeyType::kKeyTypeF23;
    case GLFW_KEY_F24:
      return KeyType::kKeyTypeF24;
    case GLFW_KEY_F25:
      return KeyType::kKeyTypeF25;
    case GLFW_KEY_KP_0:
      return KeyType::kKeyTypeKeyPad0;
    case GLFW_KEY_KP_1:
      return KeyType::kKeyTypeKeyPad1;
    case GLFW_KEY_KP_2:
      return KeyType::kKeyTypeKeyPad2;
    case GLFW_KEY_KP_3:
      return KeyType::kKeyTypeKeyPad3;
    case GLFW_KEY_KP_4:
      return KeyType::kKeyTypeKeyPad4;
    case GLFW_KEY_KP_5:
      return KeyType::kKeyTypeKeyPad5;
    case GLFW_KEY_KP_6:
      return KeyType::kKeyTypeKeyPad6;
    case GLFW_KEY_KP_7:
      return KeyType::kKeyTypeKeyPad7;
    case GLFW_KEY_KP_8:
      return KeyType::kKeyTypeKeyPad8;
    case GLFW_KEY_KP_9:
      return KeyType::kKeyTypeKeyPad9;
    case GLFW_KEY_KP_DECIMAL:
      return KeyType::kKeyTypeKeyPadDecimal;
    case GLFW_KEY_KP_DIVIDE:
      return KeyType::kKeyTypeKeyPadDivide;
    case GLFW_KEY_KP_MULTIPLY:
      return KeyType::kKeyTypeKeyPadMultiply;
    case GLFW_KEY_KP_SUBTRACT:
      return KeyType::kKeyTypeKeyPadSubtract;
    case GLFW_KEY_KP_ADD:
      return KeyType::kKeyTypeKeyPadAdd;
    case GLFW_KEY_KP_ENTER:
      return KeyType::kKeyTypeKeyPadEnter;
    case GLFW_KEY_KP_EQUAL:
      return KeyType::kKeyTypeKeyPadEqual;
    case GLFW_KEY_LEFT_SHIFT:
      return KeyType::kKeyTypeLeftShift;
    case GLFW_KEY_LEFT_CONTROL:
      return KeyType::kKeyTypeLeftControl;
    case GLFW_KEY_LEFT_ALT:
      return KeyType::kKeyTypeLeftAlt;
    case GLFW_KEY_LEFT_SUPER:
      return KeyType::kKeyTypeLeftSuper;
    case GLFW_KEY_RIGHT_SHIFT:
      return KeyType::kKeyTypeRightShift;
    case GLFW_KEY_RIGHT_CONTROL:
      return KeyType::kKeyTypeRightControl;
    case GLFW_KEY_RIGHT_ALT:
      return KeyType::kKeyTypeRightAlt;
    case GLFW_KEY_RIGHT_SUPER:
      return KeyType::kKeyTypeRightSuper;
    case GLFW_KEY_MENU:
      return KeyType::kKeyTypeMenu;
  }
  return KeyType::kKeyTypeUnknown;
}

KeyModifiers GLFWKeyModifiersToModifiers(int mods) {
  KeyModifiers modifiers = KeyModifier::kKeyModifierNone;

  if (mods & GLFW_MOD_SHIFT) {
    modifiers |= KeyModifier::kKeyModifierShift;
  }
  if (mods & GLFW_MOD_CONTROL) {
    modifiers |= KeyModifier::kKeyModifierControl;
  }
  if (mods & GLFW_MOD_ALT) {
    modifiers |= KeyModifier::kKeyModifierAlt;
  }
  if (mods & GLFW_MOD_SUPER) {
    modifiers |= KeyModifier::kKeyModifierSuper;
  }
  if (mods & GLFW_MOD_CAPS_LOCK) {
    modifiers |= KeyModifier::kKeyModifierCapsLock;
  }
  if (mods & GLFW_MOD_NUM_LOCK) {
    modifiers |= KeyModifier::kKeyModifierNumLock;
  }

  return modifiers;
}

KeyAction GLFWKeyActionToAction(int key_action) {
  switch (key_action) {
    case GLFW_RELEASE:
      return KeyAction::kKeyActionRelease;
    case GLFW_PRESS:
      return KeyAction::kKeyActionPress;
    case GLFW_REPEAT:
      return KeyAction::kKeyActionRepeat;
  }
  return KeyAction::kKeyActionUnknown;
}

}  // namespace pixel
