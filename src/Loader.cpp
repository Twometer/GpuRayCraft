//
// Created by twome on 3 Jun 2021.
//

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <lodepng.h>
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

Image Loader::load_image(const std::string &path) {
    auto image = new std::vector<uint8_t>;
    unsigned width, height;
     lodepng::decode(*image, width, height, path);
    return {&image->front(), (unsigned int) image->size(), width, height};
}

GLuint Loader::load_texture(const std::string &path) {
    Image img = load_image(path);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    delete[] img.data;

    return texture;
}

uint8_t *Loader::load_all_bytes(const std::string &path, size_t &size) {
    std::ifstream file(path, std::ios::binary | std::ios::in);
    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);

    if (length == -1)
        return nullptr;

    auto *buf = new uint8_t[length];
    file.read((char *) buf, length);
    size = length;

    file.close();
    return buf;
}

