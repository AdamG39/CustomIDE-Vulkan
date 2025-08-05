#version 450

//VERTEX SHADER

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColour;

layout(location = 0) out vec4 fragColour;

vec3 framebufferToScreenSpace(vec3 c) {
  vec3 ret = vec3(((c.x / 2560) * 2) - 1, ((c.y / 1392) * 2) - 1, c.z);
  return ret;
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
