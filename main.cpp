#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <chrono>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <vector>
#include "src/SoftwareRasterizer.hpp"
#include "src/buffers/VertexBuffer.hpp"
#include "src/buffers/IndexBuffer.hpp"

#include <glm/gtx/transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION

#include "src/tiny_obj_loader.h"

using namespace std;

void render();

void errorCallback(int error, const char *description) {
  cerr << "Error " << error << ": " << description << endl;
}

constexpr int FB_WIDTH = 1024;
constexpr int FB_HEIGHT = 1024;
//constexpr int FB_WIDTH = 512;
//constexpr int FB_HEIGHT = 512;
//constexpr int FB_WIDTH = 128;
//constexpr int FB_HEIGHT = 128;

GLuint texID;
SoftwareRasterizer *renderer;
uint64_t frame = 0;
glm::ivec2 a, b, c;

// the teapot
VertexBuffer<glm::vec3> teapotVerts;
IndexBuffer teapotIndicies;

using namespace glm;

void loadTeapot();

int main() {
  loadTeapot();

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

void render() {
  renderer->setCurrentColor({0, 0, 0, 1});
  renderer->clear();

  // matricies
  float yaw = (frame) / 200.f;
  float pitch = sin((frame) / 400.f);
  float dist = 50;

  mat4 proj = perspectiveFov(
    45.f, (float) FB_WIDTH, (float) FB_HEIGHT, 0.1f, 1000.f
  );

//  mat4 proj = ortho(-dist, dist, -dist, dist, 0.1f, 1000.f);

  mat4 view = lookAt(
    vec3{cos(pitch) * sin(yaw), cos(pitch) * cos(yaw), sin(pitch)} * dist, //eye
    vec3{0, 0, 0},// center,
    vec3{0, 0, -1}//up
  );

  mat4 model(1);

  // verts/indicies
  VertexBuffer<vec3> verts{
    {1,  1,  1},
    {-1, 1,  1},
    {1,  -1, 1},
    {-1, -1, 1},
    {1,  1,  -1},
    {-1, 1,  -1},
    {1,  -1, -1},
    {-1, -1, -1}
  };

  constexpr index_t
    ftr = 0,
    ftl = 1,
    fbr = 2,
    fbl = 3,
    btr = 4,
    btl = 5,
    bbr = 6,
    bbl = 7;

  IndexBuffer indicies{
    // front
    ftr, ftl, fbl,
    fbl, fbr, ftr,
    //back
    btr, btl, bbl,
    bbl, bbr, btr
  };

  // convert to clip space by hand for now
  mat4 mult = proj * view * model;
  vector<vec4> clipSpace;
//  clipSpace.resize(verts.size());
//  for (int i = 0; i < verts.size(); i++) {
//    clipSpace[i] = mult * vec4{verts[i], 1};
//  }
//
//  renderer->setCurrentColor({1, 0, 0, 1});
//  renderer->drawClipSpaceIndexed(
//    SoftwareRasterizer::DrawMode::TRAINGLES,
//    clipSpace,
//    indicies
//  );

  clipSpace.resize(teapotVerts.size());
  for (int i = 0; i < teapotVerts.size(); i++) {
    clipSpace[i] = mult * vec4{teapotVerts[i], 1};
  }

  renderer->setCurrentColor({0, 1, 0, 1});
  renderer->drawClipSpaceIndexed(
    SoftwareRasterizer::DrawMode::TRAINGLES,
    clipSpace,
    teapotIndicies
  );
}

void loadTeapot() {
//  string inputfile = "models/teapot.obj";
  string inputfile = "models/teapot-low.obj";
  string error;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, inputfile.c_str(), nullptr, true)) {
    cerr << error << endl;
    exit(1);
  }

  if (!error.empty()) {
    cerr << error << endl;
  }

  teapotVerts.resize(attrib.vertices.size() / 3);
  for (size_t i = 0, end = attrib.vertices.size() / 3; i < end; i++) {
    teapotVerts[i] = {
      attrib.vertices[3 * i + 0],
      attrib.vertices[3 * i + 1],
      attrib.vertices[3 * i + 2]
    };
  }

  for (auto shape_iter = shapes.cbegin(), end = shapes.cend(); shape_iter < end; shape_iter++) {
    auto shape = *shape_iter;
    auto offset = teapotIndicies.size();
    teapotIndicies.resize(teapotIndicies.size() + shape.mesh.indices.size());

    for (int i = 0, end = shape.mesh.indices.size(); i < end; i++) {
      auto index = shape.mesh.indices[i];
      teapotIndicies[offset + i] = index.vertex_index;
    }
  }

  cout << "Loaded " << teapotVerts.size() << " verts, " << teapotIndicies.size() << " indicies" << endl;
}