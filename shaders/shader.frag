#version 450

//FRAGMENT SHADER

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColour;

vec4 srgbToLinear(vec4 c) {
  return vec4((c.x <= 0.04045) ? c.x / 12.92 : pow((c.x + 0.055) / 1.055, 2.4),
              (c.y <= 0.04045) ? c.y / 12.92 : pow((c.y + 0.055) / 1.055, 2.4),
              (c.z <= 0.04045) ? c.z / 12.92 : pow((c.z + 0.055) / 1.055, 2.4),
              (c.w <= 0.04045) ? c.w / 12.92 : pow((c.w + 0.055) / 1.055, 2.4));
}


void main() {
  if (fragTexCoord.x < 0 || fragTexCoord.y < 0) {
    outColour = srgbToLinear(fragColour);
  } else {
    outColour = srgbToLinear(texture(texSampler, fragTexCoord));
  }
}
