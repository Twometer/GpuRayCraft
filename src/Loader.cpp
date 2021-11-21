//
// Created by twome on 3 Jun 2021.
//

#include <sstream>
#include <fstream>
#include <iostream>
#include "Loader.h"

void Loader::check_shader(const std::string &name, GLuint shader) {
    GLint result = 0;
    GLint logLength = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength) {
        char *data = static_cast<char *>(calloc(logLength + 1, 1));
        glGetShaderInfoLog(shader, logLength, nullptr, data);
        std::cout << "** Error in '" << name << "' **" << std::endl << data << std::endl;
        free(data);
    } else {
        std::cout << "info: Shader " << name << " compiled successfully" << std::endl;
    }

}

std::string Loader::load_file(const std::string &path) {
    std::ifstream fileStream(path, std::ios::in);

    std::stringstream readBuf;
    readBuf << fileStream.rdbuf();
    return readBuf.str();

}

Shader *Loader::load_compute_shader(const std::string &path) {
    std::cout << "info: loading compute shader " << path << std::endl;
    auto glslData = load_file(path);
    auto glslDataPtr = glslData.c_str();

    auto program = glCreateProgram();

    auto computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &glslDataPtr, nullptr);
    glCompileShader(computeShader);
    check_shader("compute", computeShader);

    glAttachShader(program, computeShader);
    glLinkProgram(program);

    glDeleteShader(computeShader);
    return new Shader(program);
}

Shader *Loader::load_draw_shader(const std::string &vert, const std::string &frag) {
    std::cout << "info: loading shader " << vert << " " << frag << std::endl;
    auto vertData = load_file(vert);
    auto vertDataPtr = vertData.c_str();
    auto fragData = load_file(frag);
    auto fragDataPtr = fragData.c_str();

    auto program = glCreateProgram();

    auto vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertDataPtr, nullptr);
    glCompileShader(vertex);
    check_shader("vert", vertex);

    auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragDataPtr, nullptr);
    glCompileShader(fragment);
    check_shader("frag", fragment);

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return new Shader(program);

}
