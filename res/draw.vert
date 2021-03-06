#version 430

in vec2 position;

out vec2 texCoords;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    texCoords = position * 0.5 + 0.5;
}