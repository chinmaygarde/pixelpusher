#include "string_utils.h"

#include <cstdarg>

namespace pixel {

std::string TrimString(const std::string& string) {
  if (string.empty()) {
    return "";
  }

  size_t start_index = 0;
  for (size_t i = 0; i < string.size(); i++) {
    if (std::isspace(string[i])) {
      start_index = i + 1;
    } else {
      break;
    }
  }

  size_t end_index = string.size();
  for (size_t i = string.size() - 1u; i > 0; i--) {
    if (std::isspace(string[i])) {
      end_index = i;
    } else {
      break;
    }
  }

  if (start_index >= end_index) {
    return "";
  }

  return std::string{string.begin() + start_index, string.begin() + end_index};
}

std::string MakeStringF(const char* format, ...) {
  va_list list;
  va_start(list, format);
  char buffer[80] = {0};
  vsnprintf(buffer, sizeof(buffer), format, list);
  va_end(list);
  return std::string{buffer};
}

}  // namespace pixel
