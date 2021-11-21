//
// Created by twome on 3 Jun 2021.
//

#ifndef MESMERIZING_SIMULATIONS_SHADER_H
#define MESMERIZING_SIMULATIONS_SHADER_H

#include <map>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
private:
    GLuint programId;
    std::map<std::string, GLint> uniformCache;

public:
    explicit Shader(GLuint programId);

    void set(const std::string &uniform, const glm::mat4 &value);
    void set(const std::string &uniform, const glm::vec2 &value);
    void set(const std::string &uniform, const glm::vec3 &value);
    void set(const std::string &uniform, const glm::vec4 &value);
    void set(const std::string &uniform, float value);
    void set(const std::string &uniform, int value);

    void bind();
    void unbind();
    void dispatch(GLuint x, GLuint y, GLuint z);

private:
    GLint get_uniform_location(const std::string &uniform);

};


#endif //MESMERIZING_SIMULATIONS_SHADER_H
