#version 330 core

out vec4 FragColor;
uniform vec4 userColor;

void main() {
    FragColor = userColor;
}
