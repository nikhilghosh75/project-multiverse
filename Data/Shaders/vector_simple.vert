#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inLayer;

layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = vec4(inPosition, inLayer, 1.0);
    fragColor = inColor;
}