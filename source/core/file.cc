#include "file.h"

#include <fstream>
#include <streambuf>

#include "logging.h"

namespace pixel {

std::vector<char> ReadFile(const char* file_name) {
  std::fstream stream(file_name, std::ios::ate | std::ios::binary);

  if (!stream.is_open()) {
    P_ERROR << "Could not open file: " << file_name;
    return {};
  }

  std::vector<char> buffer;

  stream.seekg(0, std::ios::end);
  buffer.reserve(stream.tellg());
  stream.seekg(0, std::ios::beg);

  buffer.assign(std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>());

  return buffer;
}

}  // namespace pixel
