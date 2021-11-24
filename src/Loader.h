//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_LOADER_H
#define MESMERIZING_SIMULATIONS_LOADER_H

#include <glad/glad.h>
#include <string>
#include "Shader.h"

struct Image {
    uint8_t *data;
    size_t size;
    uint32_t width;
    uint32_t height;
};

class Loader {
private:
    static void check_shader(const std::string &name, GLuint shader);

    static std::string load_file(const std::string &path);

    static Image load_image(const std::string &path);

    static uint8_t *load_all_bytes(const std::string &path, size_t &size);

public:
    static Shader *load_compute_shader(const std::string &path);

    static Shader *load_draw_shader(const std::string &vert, const std::string &frag);

    static GLuint load_texture(const std::string &path);
};


#endif //MESMERIZING_SIMULATIONS_LOADER_H
