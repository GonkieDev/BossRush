#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform GlobalUBO
{
    mat4 proj;
    mat4 cam;
};

void main()
{
    gl_Position = proj * cam * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}