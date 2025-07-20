#version 450

layout(location = 0) out vec3 fragColour;

vec2 positions[12] = vec2[](
  vec2(1, -0.95),
  vec2(1, 1),
  vec2(-1, 1),
  vec2(-1, -0.95),
  vec2(1, -0.95),
  vec2(-1, 1),

  vec2(-1, -1),
  vec2(1, -1),
  vec2(-1, -0.95),
  vec2(1, -1),
  vec2(1, -0.95),
  vec2(-1, -0.95)
);

vec3 colours[12] = vec3[](
  vec3(0.2313, 0.1098, 0.1960),
  vec3(0.2313, 0.1098, 0.1960),
  vec3(0.2313, 0.1098, 0.1960),
  vec3(0.2313, 0.1098, 0.1960),
  vec3(0.2313, 0.1098, 0.1960),
  vec3(0.2313, 0.1098, 0.1960),

  vec3(0.1012, 0.1012, 0.1137),
  vec3(0.1012, 0.1012, 0.1137),
  vec3(0.1012, 0.1012, 0.1137),
  vec3(0.1012, 0.1012, 0.1137),
  vec3(0.1012, 0.1012, 0.1137),
  vec3(0.1012, 0.1012, 0.1137)
);

float srgbToLinear(float c) {
  return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  fragColour = vec3(srgbToLinear(colours[gl_VertexIndex].x),
                    srgbToLinear(colours[gl_VertexIndex].y),
                    srgbToLinear(colours[gl_VertexIndex].z));
}
