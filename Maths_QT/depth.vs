#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    TexCoords = texCoords;
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
}  