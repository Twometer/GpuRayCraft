#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"
#include "Loader.h"
#include <SimplexNoise.h>
#include <iostream>

#define WIDTH 1920
#define HEIGHT 1080

#define GROUP_SIZE 30
#define WORLD_SIZE_X 512
#define WORLD_SIZE_Y 64
#define WORLD_SIZE_Z 512
#define WORLD_SIZE   (WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z)

#define SEA_LEVEL 48

Shader *drawShader;
Shader *workShader;
GLuint drawTexture;
GLuint fullscreenQuad;
GLuint terrainTexture;
GLuint ssbo;
uint32_t *voxel_data;
GLFWwindow *window;

glm::vec3 cameraPos = glm::vec3(0, 64, 0);
glm::vec2 cameraRot = glm::vec2(-0.5, 0);
glm::mat4 cameraMat = glm::mat4();
glm::vec3 lightDir = glm::vec3(0, 1, 0);
float curTime = 0;

SimplexNoise noise{};

void generate_world() {
    float scale = 0.015f;
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            int height = (WORLD_SIZE_Y - SEA_LEVEL) * noise.fractal(4, float(x) * scale, float(z) * scale);
            for (int y = 0; y < WORLD_SIZE_Y; y++) {

                int idx = x + WORLD_SIZE_X * (y + WORLD_SIZE_Y * z);
                if (y < SEA_LEVEL + height)
                    voxel_data[idx] = 1;
                else
                    voxel_data[idx] = 0;

                if (y < SEA_LEVEL)
                    voxel_data[idx] = 1;
            }
        }
    }
}

void init() {
    // load shader
    drawShader = Loader::load_draw_shader("res/draw.vert", "res/draw.frag");
    workShader = Loader::load_compute_shader("res/raytracer.glsl");

    // setup draw texture
    glGenTextures(1, &drawTexture);
    glBindTexture(GL_TEXTURE_2D, drawTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, drawTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // load world texture
    terrainTexture = Loader::load_texture("res/stone.png");

    // setup voxel world
    voxel_data = new uint32_t[WORLD_SIZE];
    generate_world();

    // setup shader storage
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, WORLD_SIZE * sizeof(uint32_t), voxel_data, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    // setup screen quad
    glGenVertexArrays(1, &fullscreenQuad);
    glBindVertexArray(fullscreenQuad);

    GLuint posBuf;
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    float data[] = {
            -1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    lightDir = glm::vec3(0, glm::sin(curTime), glm::cos(curTime));
}

void camera_update() {
    static double lastTime = glfwGetTime();
    double deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);
    glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

    double dx = (WIDTH / 2) - cx;
    double dy = (HEIGHT / 2) - cy;


    cameraRot.x += dx * 0.1 * deltaTime;
    cameraRot.y += dy * 0.1 * deltaTime;

    cameraMat = glm::mat4(glm::quat(glm::vec3(cameraRot.y, cameraRot.x, 0)));

    float speed = 20.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed = 50.0 * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 vec = glm::vec3(
                glm::sin(cameraRot.x),
                0,
                glm::cos(cameraRot.x)
        ) * -speed;
        cameraPos += vec;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 vec = glm::vec3(
                glm::cos(cameraRot.x),
                0,
                -glm::sin(cameraRot.x)
        ) * -speed;
        cameraPos += vec;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 vec = glm::vec3(
                glm::cos(cameraRot.x),
                0,
                -glm::sin(cameraRot.x)
        ) * speed;
        cameraPos += vec;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 vec = glm::vec3(
                glm::sin(cameraRot.x),
                0,
                glm::cos(cameraRot.x)
        ) * speed;
        cameraPos += vec;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPos.y += speed;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos.y -= speed;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        curTime += 0.01;
        lightDir = glm::vec3(0, glm::sin(curTime), glm::cos(curTime));
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        curTime -= 0.01;
        lightDir = glm::vec3(0, glm::sin(curTime), glm::cos(curTime));
    }
}

void draw() {
    camera_update();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);
    workShader->bind();
    workShader->set("screenSize", glm::vec2(WIDTH, HEIGHT));
    workShader->set("cameraMatrix", cameraMat);
    workShader->set("cameraPos", cameraPos);
    workShader->set("lightDir", lightDir);
    workShader->dispatch(WIDTH / GROUP_SIZE, HEIGHT / GROUP_SIZE, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drawTexture);

    drawShader->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


double prevtime;
int frames;


int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "GPU Voxel Raytracer", nullptr, nullptr);
    if (!window) {
        glfwDestroyWindow(window);
        return 1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwPollEvents();
    glfwSwapInterval(1);

    init();
    while (!glfwWindowShouldClose(window)) {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        draw();

        frames++;
        if (glfwGetTime() - prevtime > 1) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            prevtime = glfwGetTime();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
