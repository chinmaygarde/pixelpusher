#include "shader_loader.h"

#include <sstream>

#include "shader_location.h"

namespace pixel {

std::vector<char> LoadShader(const char* shader_name) {
  std::stringstream stream;
  stream << PIXEL_SHADERS_LOCATION << "/" << shader_name;
  return ReadFile(stream.str().c_str());
}

}  // namespace pixel
