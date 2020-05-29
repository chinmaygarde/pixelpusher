#pragma once

#include <string>

namespace pixel {

std::string GetLastErrorMessage();

std::string WideStringToString(const std::wstring& string);

}  // namespace pixel
