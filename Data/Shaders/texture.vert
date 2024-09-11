#version 450
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main() 
{
    fragTexCoord = inTexCoord;
    vec2 position = inPos * 2.0 - vec2(1.0, 1.0);
    gl_Position = vec4(position, 0.0, 1.0);
}