#include "win_utils.h"

#include <sstream>

#include <Windows.h>

namespace pixel {

std::string GetLastErrorMessage() {
  DWORD last_error = ::GetLastError();
  if (last_error == 0) {
    return {};
  }

  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS;

  LPSTR buffer = nullptr;
  size_t size = ::FormatMessageA(
      flags,                                      // dwFlags
      NULL,                                       // lpSource
      last_error,                                 // dwMessageId
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // dwLanguageId
      (LPSTR)&buffer,                             // lpBuffer
      0,                                          // nSize
      NULL                                        // Arguments
  );

  std::string message(buffer, size);

  ::LocalFree(buffer);

  std::stringstream stream;
  stream << "Error: (" << last_error << "): " << message;

  return stream.str();
}

}  // namespace pixel
