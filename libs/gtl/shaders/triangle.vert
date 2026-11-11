#version 330 core
layout (location = 0) in vec2 aPos; // vec2 instead of vec3

uniform mat4 projection; // Maps pixels (0..800, 0..600) to OpenGL space
uniform mat4 model;      // Moves and resizes the rectangle

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}
