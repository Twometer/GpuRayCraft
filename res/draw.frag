#version 430

out vec4 color;

in vec2 texCoords;

uniform sampler2D textureSampler;

void main() {
    vec3 rgb = texture(textureSampler, texCoords).rgb;
    color = vec4(rgb, 1.0);
}