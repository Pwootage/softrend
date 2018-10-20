#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <chrono>
#include <numeric>
#include <sstream>
#include <iomanip>
#include "SoftwareRasterizer.hpp"

using namespace std;

void render();

void errorCallback(int error, const char *description) {
  cerr << "Error " << error << ": " << description << endl;
}

//constexpr int FB_WIDTH = 512;
//constexpr int FB_HEIGHT = 512;
constexpr int FB_WIDTH = 128;
constexpr int FB_HEIGHT = 128;

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
//  glfwSwapInterval(1);

  constexpr int FRAME_AVG_COUNT = 60;
  double frameTimes[FRAME_AVG_COUNT];
  for (int i = 0; i < FRAME_AVG_COUNT; i++) {
    frameTimes[i] = 1;
  }
  glfwSwapInterval(0);
  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    {
      auto startT = chrono::high_resolution_clock::now();
      render();
      auto endT = chrono::high_resolution_clock::now();
      auto time = chrono::duration<double, milli>(endT - startT).count();
      frameTimes[frame % FRAME_AVG_COUNT] = time;
    }
    auto avg = accumulate(begin(frameTimes), end(frameTimes), 0.0) / (double) FRAME_AVG_COUNT;
    stringstream title;
    title << "softrend " << fixed << setprecision(2) << avg << " ms " << frame;
    string titleS = title.str();
    glfwSetWindowTitle(window, titleS.c_str());
    frame++;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 renderer->getWidth(), renderer->getHeight(),
                 0, GL_RGBA, GL_FLOAT,
                 renderer->getFramebuffer());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);

    glColor4f(1, 1, 1, 1);

    glTexCoord2f(0, 1);
    glVertex3f(-1, -1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-1, 1, 0);

    glTexCoord2f(1, 0);
    glVertex3f(1, 1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1, -1, 0);

    glEnd();
    glfwSwapBuffers(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void drawTri(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c, color_t color) {
  renderer->setCurrentColor(color);
  renderer->drawTriFilled(a, b, c);

//  renderer->setCurrentColor(0xFF0000FF);
//  renderer->drawTriLines(a, b, c);
}

void render() {
  renderer->setCurrentColor({1, 1, 0, 1});

  for (int i = 0; i < 3000; i++) {
//  if (frame % 300 == 0) {
    a = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
    b = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
    c = {rand() % FB_WIDTH, rand() % FB_HEIGHT};

//  }

    drawTri(a, b, c, {
      rand() % 256 / 256.f,
      rand() % 256 / 256.f,
      rand() % 256 / 256.f,
      1
    });
  }

//  renderer->setCurrentColor({0, 0, 0, 1});
//  renderer->clear();

  renderer->setCurrentColor({0, 1, 0, 1});
  renderer->drawLine({0, 0}, {0, FB_HEIGHT - 1});
  renderer->drawLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, FB_HEIGHT - 1});
  renderer->drawLine({FB_WIDTH - 1, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});
  renderer->drawLine({FB_WIDTH - 1, 0}, {0, 0});

  renderer->setCurrentColor({1, 0, 0, 1});
  renderer->drawLine({0, 0}, {FB_WIDTH - 1, FB_HEIGHT - 1});
  renderer->drawLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});

//  drawTri(
//    {0, 0},
//    {0, 100},
//    {100, 0},
//    0xFF00FFFF
//  );
//
//  drawTri(
//    {FB_WIDTH - 1, FB_HEIGHT - 1},
//    {FB_WIDTH - 1, FB_HEIGHT - 101},
//    {FB_WIDTH - 101, FB_HEIGHT - 1},
//    0xFFFF00FF
//  );
//
//
//  drawTri(
//    {100, 100},
//    {150, 192},
//    {174, 140},
//    0xFFFFFFFF
//  );
}