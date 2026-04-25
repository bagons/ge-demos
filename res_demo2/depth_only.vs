#version 330 core
layout (location = 0) in vec3 VERTEX;

uniform mat4 transform;
layout (std140) uniform LIGHT_MATRICES
{
    mat4 light_space_mat;
};

void main()
{
    gl_Position = light_space_mat * transform * vec4(VERTEX, 1.0);
}