#version 450

//FRAGMENT SHADER

layout(set = 0, binding = 0) uniform sampler samp;
layout(set = 0, binding = 1) uniform texture2D textures[100];

layout(push_constant) uniform PushConstants {
  layout(offset = 8) int textureIndex;
} pc;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColour;

vec4 srgbToLinear(vec4 c) {
  return vec4((c.x <= 0.04045) ? c.x / 12.92 : pow((c.x + 0.055) / 1.055, 2.4),
              (c.y <= 0.04045) ? c.y / 12.92 : pow((c.y + 0.055) / 1.055, 2.4),
              (c.z <= 0.04045) ? c.z / 12.92 : pow((c.z + 0.055) / 1.055, 2.4),
              c.w);
}


void main() {
  if (pc.textureIndex < 0) {
    outColour = srgbToLinear(fragColour);
  } else {
    outColour = srgbToLinear(fragColour * 
                texture(sampler2D(textures[pc.textureIndex], samp), fragTexCoord));
  }
}
