//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_LOADER_H
#define MESMERIZING_SIMULATIONS_LOADER_H


#include <glad/glad.h>
#include <string>
#include "Shader.h"

class Loader {
private:
    static void check_shader(const std::string &name, GLuint shader);

    static std::string load_file(const std::string &path);

public:
    static Shader *load_compute_shader(const std::string &path);

    static Shader *load_draw_shader(const std::string &vert, const std::string &frag);

};


#endif //MESMERIZING_SIMULATIONS_LOADER_H
