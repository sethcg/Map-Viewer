#version 450 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord;

uniform mat4 uVP;
uniform mat4 uModel;

out vec2 uv;

void main() {
    uv = texcoord;
    gl_Position = uVP * uModel * vec4(position, 0.0, 1.0);
}