#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 outColor;
out vec2 outTextureCoordinate;
out vec3 outPosition;
out vec3 outNormal;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);;
    outColor = color;
    outTextureCoordinate = vec2(tex_coord.x, 1 - tex_coord.y);
    outNormal = normal;
    outPosition = vec3(model * vec4(position, 1.0));
}