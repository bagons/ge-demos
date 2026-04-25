#version 330 core

layout (location = 0) in vec3 VERTEX_POS;
layout (location = 1) in vec2 TEXTURE_COORDS;

uniform mat4 transform;

out vec2 UV;

void main(){
    gl_Position = vec4(VERTEX_POS.x, VERTEX_POS.y, 0.0, 1.0);
    UV = TEXTURE_COORDS;
}