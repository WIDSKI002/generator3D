#version 330 core
layout(location = 0) in vec3 aPos; // Pozycja wierzchołka

out float Height; // Przekazujemy wysokość do fragment shadera

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Height = aPos.y; // Przekazujemy wysokość (składową y)
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}