#pragma once

#include <set>

#include "macros.h"

namespace pixel {

enum class KeyType {
  kKeyTypeUnknown,
  kKeyTypeSpace,
  kKeyTypeApostrophe,
  kKeyTypeComma,
  kKeyTypeMinus,
  kKeyTypePeriod,
  kKeyTypeSlash,
  kKeyType0,
  kKeyType1,
  kKeyType2,
  kKeyType3,
  kKeyType4,
  kKeyType5,
  kKeyType6,
  kKeyType7,
  kKeyType8,
  kKeyType9,
  kKeyTypeSemicolon,
  kKeyTypeEqual,
  kKeyTypeA,
  kKeyTypeB,
  kKeyTypeC,
  kKeyTypeD,
  kKeyTypeE,
  kKeyTypeF,
  kKeyTypeG,
  kKeyTypeH,
  kKeyTypeI,
  kKeyTypeJ,
  kKeyTypeK,
  kKeyTypeL,
  kKeyTypeM,
  kKeyTypeN,
  kKeyTypeO,
  kKeyTypeP,
  kKeyTypeQ,
  kKeyTypeR,
  kKeyTypeS,
  kKeyTypeT,
  kKeyTypeU,
  kKeyTypeV,
  kKeyTypeW,
  kKeyTypeX,
  kKeyTypeY,
  kKeyTypeZ,
  kKeyTypeLeftBracket,
  kKeyTypeBackslash,
  kKeyTypeRightBracket,
  kKeyTypeGraveAccent,
  kKeyTypeWorld1,
  kKeyTypeWorld2,
  kKeyTypeEscape,
  kKeyTypeEnter,
  kKeyTypeTab,
  kKeyTypeBackspace,
  kKeyTypeInsert,
  kKeyTypeDelete,
  kKeyTypeRight,
  kKeyTypeLeft,
  kKeyTypeDown,
  kKeyTypeUp,
  kKeyTypePageUp,
  kKeyTypePageDown,
  kKeyTypeHome,
  kKeyTypeEnd,
  kKeyTypeCapsLock,
  kKeyTypeScrollLock,
  kKeyTypeNumLock,
  kKeyTypePrintScreen,
  kKeyTypePause,
  kKeyTypeF1,
  kKeyTypeF2,
  kKeyTypeF3,
  kKeyTypeF4,
  kKeyTypeF5,
  kKeyTypeF6,
  kKeyTypeF7,
  kKeyTypeF8,
  kKeyTypeF9,
  kKeyTypeF10,
  kKeyTypeF11,
  kKeyTypeF12,
  kKeyTypeF13,
  kKeyTypeF14,
  kKeyTypeF15,
  kKeyTypeF16,
  kKeyTypeF17,
  kKeyTypeF18,
  kKeyTypeF19,
  kKeyTypeF20,
  kKeyTypeF21,
  kKeyTypeF22,
  kKeyTypeF23,
  kKeyTypeF24,
  kKeyTypeF25,
  kKeyTypeKeyPad0,
  kKeyTypeKeyPad1,
  kKeyTypeKeyPad2,
  kKeyTypeKeyPad3,
  kKeyTypeKeyPad4,
  kKeyTypeKeyPad5,
  kKeyTypeKeyPad6,
  kKeyTypeKeyPad7,
  kKeyTypeKeyPad8,
  kKeyTypeKeyPad9,
  kKeyTypeKeyPadDecimal,
  kKeyTypeKeyPadDivide,
  kKeyTypeKeyPadMultiply,
  kKeyTypeKeyPadSubtract,
  kKeyTypeKeyPadAdd,
  kKeyTypeKeyPadEnter,
  kKeyTypeKeyPadEqual,
  kKeyTypeLeftShift,
  kKeyTypeLeftControl,
  kKeyTypeLeftAlt,
  kKeyTypeLeftSuper,
  kKeyTypeRightShift,
  kKeyTypeRightControl,
  kKeyTypeRightAlt,
  kKeyTypeRightSuper,
  kKeyTypeMenu,
};

enum class KeyAction {
  kKeyActionUnknown,
  kKeyActionRelease,
  kKeyActionPress,
  kKeyActionRepeat,
};

enum KeyModifier : uint32_t {
  kKeyModifierNone = 0,
  kKeyModifierShift = 1 << 0,
  kKeyModifierControl = 1 << 1,
  kKeyModifierAlt = 1 << 2,
  kKeyModifierSuper = 1 << 3,
  kKeyModifierCapsLock = 1 << 4,
  kKeyModifierNumLock = 1 << 5,
};

using KeyModifiers = uint32_t;

class KeyInputDelegate {
 public:
  virtual bool WantsKeyEvents() = 0;

  virtual void OnKeyEvent(KeyType type,
                          KeyAction action,
                          KeyModifiers modifier) = 0;
};

class KeyInputDispatcher {
 public:
  KeyInputDispatcher();

  ~KeyInputDispatcher();

  bool AddDelegate(KeyInputDelegate* delegate);

  bool RemoveDelegate(KeyInputDelegate* delegate);

  void DispatchKey(KeyType type, KeyAction action, KeyModifiers modifier);

 private:
  std::set<KeyInputDelegate*> delegates_;

  P_DISALLOW_COPY_AND_ASSIGN(KeyInputDispatcher);
};

}  // namespace pixel
