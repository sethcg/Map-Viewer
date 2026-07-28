#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in float polygonID;

out float vPolygonID;

uniform mat4 projection;

void main()
{
    vPolygonID = polygonID;
    gl_Position = projection * vec4(position, 0.0, 1.0);
}