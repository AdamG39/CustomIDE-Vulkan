#version 450

//FRAGMENT SHADER

layout(location = 0) in vec4 fragColour;

layout(location = 0) out vec4 outColour;

void main() {
  outColour = fragColour;
}
