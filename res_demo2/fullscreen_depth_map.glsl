#version 330 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D depth_map;

void main()
{
    float depth = texture(depth_map, UV).r;
    FragColor = vec4(vec3(depth), 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}