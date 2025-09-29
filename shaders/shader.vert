#version 450

//VERTEX SHADER

layout(push_constant) uniform PushConstants {
  vec2 extent;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColour;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColour;
layout(location = 1) out vec2 fragTexCoord;

vec3 framebufferToScreenSpace(vec3 c) {
  return vec3(((c.x / pc.extent.x) * 2.0) - 1.0, ((c.y / pc.extent.y) * 2.0) - 1.0, c.z);
}

void main() {
  gl_Position = vec4(framebufferToScreenSpace(inPosition), 1.0);
  fragColour = inColour;
  fragTexCoord = inTexCoord;
}
