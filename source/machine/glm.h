#pragma once

#include "vulkan.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_UNRESTRICTED_GENTYPE

#include <glm/ext.hpp>
#include <glm/glm.hpp>

template <class T>
vk::Format ToVKFormat();

template <>
constexpr vk::Format ToVKFormat<glm::vec4>() {
  return vk::Format::eR32G32B32A32Sfloat;
}

template <>
constexpr vk::Format ToVKFormat<glm::vec3>() {
  return vk::Format::eR32G32B32Sfloat;
}

template <>
constexpr vk::Format ToVKFormat<glm::vec2>() {
  return vk::Format::eR32G32Sfloat;
}
