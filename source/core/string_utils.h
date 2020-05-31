#pragma once

#include <string>

#include "macros.h"

namespace pixel {

std::string TrimString(const std::string& string);

P_PRINTF_FORMAT(1, 2)
std::string MakeStringF(const char* format, ...);

}  // namespace pixel
