#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextCoord;

layout(location = 0) out vec2 fragTextCoord;

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragTextCoord = inTextCoord;
}