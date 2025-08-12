#version 450

//VERTEX SHADER

layout(push_constant) uniform PushConstants {
  vec2 extent;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColour;

layout(location = 0) out vec4 fragColour;

vec3 framebufferToScreenSpace(vec3 c) {
  return vec3(((c.x / pc.extent.x) * 2.0) - 1.0, ((c.y / pc.extent.y) * 2.0) - 1.0, c.z);
}

float srgbToLinear(float c) {
  return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

void main() {
  gl_Position = vec4(framebufferToScreenSpace(inPosition), 1.0);
  fragColour = vec4(srgbToLinear(inColour.x),
                    srgbToLinear(inColour.y),
                    srgbToLinear(inColour.z),
                    inColour.w);
}
