#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <chrono>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <vector>
#include <random>
#include "src/SoftwareRasterizer.hpp"
#include "src/buffers/VertexBuffer.hpp"
#include "src/buffers/IndexBuffer.hpp"
#include "src/shader/VertexTypes.hpp"
#include "src/shader/BasicVertexShader.hpp"
#include "src/shader/PhongFragmentShader.hpp"
#include "src/renderTeapot.hpp"

#include <glm/gtx/transform.hpp>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

using namespace std;
using namespace softrend;
using namespace glm;

void errorCallback(int error, const char *description) {
  cerr << "Error " << error << ": " << description << endl;
}

GLuint texID;
uint64_t frame = 0;

GLFWwindow *window;
constexpr int FRAME_AVG_COUNT = 60;
double frameTimes[FRAME_AVG_COUNT];

void mainLoop();

int main() {
  teapotRender::init();

  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    cerr << "Failed to init glfw" << endl;
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  window = glfwCreateWindow(1024, 1024, "softrend", nullptr, nullptr);
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

  for (int i = 0; i < FRAME_AVG_COUNT; i++) {
    frameTimes[i] = 1;
  }
  glfwSwapInterval(1);
#ifdef EMSCRIPTEN
  emscripten_set_main_loop(mainLoop, 0, 1);
#else
  while (!glfwWindowShouldClose(window)) {
    mainLoop();
  }
#endif

  glfwTerminate();
  return 0;
}

void mainLoop() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  {
    auto startT = chrono::steady_clock::now();
    teapotRender::render(frame);
    auto endT = chrono::steady_clock::now();
    auto time = chrono::duration<double, milli>(endT - startT).count();
    frameTimes[frame % FRAME_AVG_COUNT] = time;
  }
  auto avg = accumulate(begin(frameTimes), end(frameTimes), 0.0) / (double) FRAME_AVG_COUNT;
  stringstream title;
  title << "softrend " << fixed << setprecision(2) << avg << " ms " << frame;
  string titleS = title.str();
  glfwSetWindowTitle(window, titleS.c_str());
  frame++;
  if (frame % 60 == 0) {
    cout << titleS << endl;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               teapotRender::getFBWidth(), teapotRender::getFBHeight(),
               0, GL_RGBA, GL_FLOAT,
               teapotRender::getFB());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_QUADS);

  glColor4f(1, 1, 1, 1);

  glTexCoord2f(0, 1);
  glVertex3f(-1, -1, 0);

  glColor4f(1, 1, 1, 1);
  glTexCoord2f(0, 0);
  glVertex3f(-1, 1, 0);

  glColor4f(1, 1, 1, 1);
  glTexCoord2f(1, 0);
  glVertex3f(1, 1, 0);

  glColor4f(1, 1, 1, 1);
  glTexCoord2f(1, 1);
  glVertex3f(1, -1, 0);

  glEnd();
  glfwSwapBuffers(window);

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  glfwPollEvents();
}
