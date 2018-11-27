#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <chrono>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <vector>
#include "SoftwareRasterizer.hpp"

#include <glm/gtx/transform.hpp>

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

using namespace glm;

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
  glfwSwapInterval(1);
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
  renderer->drawScreenSpaceTriFilled(a, b, c);

//  renderer->setCurrentColor(0xFF0000FF);
//  renderer->drawScreenSpaceTriLines(a, b, c);
}

void render() {
  renderer->setCurrentColor({0, 0, 0, 1});
  renderer->clear();

  renderer->setCurrentColor({1, 0, 0, 1});

  vec3 ftr{1, 1, 1};
  vec3 ftl{-1, 1, 1};
  vec3 fbr{1, -1, 1};
  vec3 fbl{-1, -1, 1};
  vec3 btr{1, 1, -1};
  vec3 btl{-1, 1, -1};
  vec3 bbr{1, -1, -1};
  vec3 bbl{-1, -1, -1};

  vector<vec3> tris{
    // front
    ftr, ftl, fbl,
    fbl, fbr, ftr,
    //back
    btr, btl, bbl,
    bbl, bbr, btr
  };

  mat4 proj = perspectiveFov(
    45.f, (float) FB_WIDTH, (float) FB_HEIGHT, 0.1f, 1000.f
  );

  float yaw = (frame) / 200.f;
  float pitch = sin((frame) / 400.f);
  float dist = 10;

  mat4 view = lookAt(
    vec3{cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw)} * dist, //eye
    vec3{0, 0, 0},// center,
    vec3{0, 1, 0}//up
  );

  mat4 model(1);

  vector<vec4> clipSpace;
  clipSpace.reserve(tris.size());

  // convert to clip space by hand for now
  for (int i = 0; i < tris.size(); i++) {
    clipSpace[i] = proj * view * model * vec4{tris[i], 1};
  }

  for (int i = 0; i < tris.size(); i += 3) {
    renderer->drawClipSpaceTriangle(
      clipSpace[i + 0],
      clipSpace[i + 1],
      clipSpace[i + 2]
    );
  }



//  renderer->setCurrentColor({1, 1, 0, 1});

//  for (int i = 0; i < 3000; i++) {
////  if (frame % 300 == 0) {
//    a = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
//    b = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
//    c = {rand() % FB_WIDTH, rand() % FB_HEIGHT};
//
////  }
//
//    drawTri(a, b, c, {
//      rand() % 256 / 256.f,
//      rand() % 256 / 256.f,
//      rand() % 256 / 256.f,
//      1
//    });
//  }
//
////  renderer->setCurrentColor({0, 0, 0, 1});
////  renderer->clear();
//
//  renderer->setCurrentColor({0, 1, 0, 1});
//  renderer->drawScreenSpaceLine({0, 0}, {0, FB_HEIGHT - 1});
//  renderer->drawScreenSpaceLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, FB_HEIGHT - 1});
//  renderer->drawScreenSpaceLine({FB_WIDTH - 1, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});
//  renderer->drawScreenSpaceLine({FB_WIDTH - 1, 0}, {0, 0});
//
//  renderer->setCurrentColor({1, 0, 0, 1});
//  renderer->drawScreenSpaceLine({0, 0}, {FB_WIDTH - 1, FB_HEIGHT - 1});
//  renderer->drawScreenSpaceLine({0, FB_HEIGHT - 1}, {FB_WIDTH - 1, 0});
//
////  drawTri(
////    {0, 0},
////    {0, 100},
////    {100, 0},
////    0xFF00FFFF
////  );
////
////  drawTri(
////    {FB_WIDTH - 1, FB_HEIGHT - 1},
////    {FB_WIDTH - 1, FB_HEIGHT - 101},
////    {FB_WIDTH - 101, FB_HEIGHT - 1},
////    0xFFFF00FF
////  );
////
////
////  drawTri(
////    {100, 100},
////    {150, 192},
////    {174, 140},
////    0xFFFFFFFF
////  );
}
