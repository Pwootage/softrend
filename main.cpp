#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include "SoftwareRasterizer.hpp"

using namespace std;

void render();

void errorCallback(int error, const char *description) {
  cerr << "Error " << error << ": " << description << endl;
}

constexpr int FB_WIDTH = 512;
constexpr int FB_HEIGHT = 512;

GLuint texID;
SoftwareRasterizer *renderer;
uint64_t frame = 0;
glm::ivec2 a, b, c;

int main() {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    cerr << "Failed to init glfw" << endl;
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  GLFWwindow *window = glfwCreateWindow(1024, 1024, "softrend", nullptr, nullptr);
  if (!window) {
    cerr << "Failed to create window" << endl;
    return 1;
  }

  glfwMakeContextCurrent(window);

  // init gl
  glClearColor(0, 0, 0, 1);
  glEnable(GL_COLOR);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE);

  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FB_WDITH, FB_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);

  renderer = new SoftwareRasterizer(FB_WIDTH, FB_HEIGHT);

  // main loop
  glfwSwapInterval(1);
  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    render();
    frame++;
    glfwSwapBuffers(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void render() {
  renderer->setCurrentColor(0xFF000000);
  renderer->clear();

  renderer->setCurrentColor(0xFF00FF00);
  renderer->drawLine({0, 0}, {0, FB_HEIGHT - 1});
  renderer->drawLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, FB_HEIGHT - 1});
  renderer->drawLine({FB_WIDTH - 1, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});
  renderer->drawLine({FB_WIDTH - 1, 0}, {0, 0});

  renderer->setCurrentColor(0xFFFF0000);
  renderer->drawLine({0, 0}, {FB_WIDTH - 1, FB_HEIGHT - 1});
  renderer->drawLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});

  renderer->setCurrentColor(0xFFFFFF00);

  if (frame % 30 == 0) {
    a = {rand() % FB_WIDTH - 100, rand() % FB_HEIGHT};
    b = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
    c = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
  }

  renderer->setCurrentColor(0xFF00FFFF);
  renderer->drawTriFilled(a, b, c);

  renderer->setCurrentColor(0xFF0000FF);
  renderer->drawTriLines(a, b, c);



  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               renderer->getWidth(), renderer->getHeight(),
               0, GL_RGBA, GL_UNSIGNED_BYTE,
               renderer->getFramebuffer());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_QUADS);

  glColor4f(1, 1, 1, 1);

  glTexCoord2f(0, 0);
  glVertex3f(-1, -1, 0);
  glTexCoord2f(0, 1);
  glVertex3f(-1, 1, 0);

  glTexCoord2f(1, 1);
  glVertex3f(1, 1, 0);
  glTexCoord2f(1, 0);
  glVertex3f(1, -1, 0);

  glEnd();
}