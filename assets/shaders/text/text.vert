#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 TexCoord;

uniform mat4 uProj;
uniform vec2 uTexSize;

void main() {
    TexCoord = aUV * uTexSize;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}