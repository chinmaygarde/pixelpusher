#version 450
#extension GL_ARB_separate_shader_objects : enable

// Uniforms

layout(binding = 1) uniform sampler2D uTextureSampler;

// In

layout(location = 0) in vec2 inTextureCoords;

// Out

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(1.0);
}
