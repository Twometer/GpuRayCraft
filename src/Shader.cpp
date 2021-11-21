//
// Created by twome on 3 Jun 2021.
//

#include "Shader.h"

Shader::Shader(GLuint programId)
        : programId(programId) {
}

GLint Shader::get_uniform_location(const std::string &uniform) {
    if (uniformCache.find(uniform) == uniformCache.end()) {
        auto loc = glGetUniformLocation(programId, uniform.c_str());
        uniformCache[uniform] = loc;
        return loc;
    } else {
        return uniformCache[uniform];
    }
}

void Shader::set(const std::string &uniform, const glm::mat4 &value) {
    glUniformMatrix4fv(get_uniform_location(uniform), 1, false, &value[0][0]);
}

void Shader::set(const std::string &uniform, const glm::vec2 &value) {
    glUniform2f(get_uniform_location(uniform), value.x, value.y);
}

void Shader::set(const std::string &uniform, const glm::vec3 &value) {
    glUniform3f(get_uniform_location(uniform), value.x, value.y, value.z);
}

void Shader::set(const std::string &uniform, const glm::vec4 &value) {
    glUniform4f(get_uniform_location(uniform), value.x, value.y, value.z, value.w);
}

void Shader::bind() {
    glUseProgram(programId);
}

void Shader::unbind() {
    glUseProgram(0);
}

void Shader::dispatch(GLuint x, GLuint y, GLuint z) {
    glDispatchCompute(x, y, z);
}

void Shader::set(const std::string &uniform, float value) {
    glUniform1f(get_uniform_location(uniform), value);
}

void Shader::set(const std::string &uniform, int value) {
    glUniform1i(get_uniform_location(uniform), value);
}
