#version 450 core

in float vPolygonID;

out vec4 FragColor;

vec3 randomColor(float id) {
    float r = fract(sin(id * 12.9898) * 43758.5453);
    float g = fract(sin(id * 78.233) * 43758.5453);
    float b = fract(sin(id * 39.425) * 43758.5453);

    return vec3(r, g, b);
}

void main()
{
    FragColor = vec4(randomColor(vPolygonID), 1.0);
}