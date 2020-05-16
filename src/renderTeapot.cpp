#include <iostream>
#include <random>

#include "renderTeapot.hpp"
#include "softrend/VertexTypes.hpp"
#include "shader/BasicVertexShader.hpp"
#include "shader/PhongFragmentShader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"

using namespace softrend;
using namespace glm;
using namespace std;

namespace renderTeapot {



SoftwareRasterizer<formats::Pos4ColorNormalTex, formats::Pos4ColorNormalTex, f32_color_t> *renderer;
BasicVertexShader basicVertShader;
PhongFragmentShader phongFragmentShader;
VertexBuffer<formats::Pos4ColorNormalTex> teapotVerts;
IndexBuffer teapotIndicies;

void loadTeapot(const char *modelPath);

void loadTeapotFromSrc(const char *modelSrc, size_t modelLen);

void init(const InitData &initData) {
  if (initData.modelPath) {
    loadTeapot(initData.modelPath);
  } else {
    loadTeapotFromSrc(initData.modelSrc, initData.modelLen);
  }

  initData.framebuffer->clear({0.f,0.f,0.f,0.f}, true, true);
  renderer = new SoftwareRasterizer<
      formats::Pos4ColorNormalTex,
      formats::Pos4ColorNormalTex,
      f32_color_t
      >(initData.framebuffer);
  renderer->vertexShader = &basicVertShader;
  renderer->fragmentShader = &phongFragmentShader;
}

void render(size_t frame) {
  // matricies
  float yaw = (frame) / 200.f;
  float pitch = sin((frame) / 400.f);
  float dist = 20;

  phongFragmentShader.light_dir = normalize(vec3{0.0, -0.5f, 0.5f});
  phongFragmentShader.light_color_diffuse = {0.8f, 0.8f, 0.8f, 1.f};
  phongFragmentShader.light_color_ambient = {0.4f, 0.4f, 0.4f, 1.f};

  yaw = 3.f * glm::pi<float>() / 4.f;
  pitch = 0.4f;
//    pitch = 0;

  // mat4 proj = perspectiveFov(
  //   45.f, (float) FB_WIDTH, (float) FB_HEIGHT, 0.1f, 1000.f
  // );

  mat4 proj = ortho(-dist, dist, -dist, dist, 0.1f, 1000.f);

  mat4 view = lookAt(
    vec3{cos(pitch) * sin(yaw), cos(pitch) * cos(yaw), sin(pitch)} * dist, //eye
    vec3{0, 0, 0},// center,
    vec3{0, 0, -1}//up
  );

  mat4 model(1);

  basicVertShader.setMVP(model, view, proj);

  // convert to clip space by hand for now
  // mat4 mult = proj * view * model;
  // vector<formats::Pos4ColorNormalTex> clipSpace;
  //
  // clipSpace.resize(teapotVerts.size());
  // for (int i = 0; i < teapotVerts.size(); i++) {
  //   clipSpace[i] = teapotVerts[i];
  //   clipSpace[i].pos = mult * clipSpace[i].pos;
  // }

//  renderer->setCurrentColor({0, 0, 0, 0});
//  renderer->clear(true, true);

  renderer->setCurrentColor({0, 1, 0, 1});
  renderer->drawIndexed(
    DrawMode::TRAINGLES,
    teapotVerts,
    teapotIndicies
  );
}

void processTeapot(const tinyobj::attrib_t &attrib, const vector<tinyobj::shape_t> &shapes);

void loadTeapot(const char *modelPath) {
  string inputfile = modelPath;
//  string inputfile = "models/teapot.obj";
//  string inputfile = "models/teapot_low.obj";
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

  processTeapot(attrib, shapes);
}

void loadTeapotFromSrc(const char *modelSrc, size_t modelLen) {
  struct membuf : std::streambuf {
    membuf(char const *base, size_t size) {
      char *p(const_cast<char *>(base));
      this->setg(p, p, p + size);
    }
  };
  struct imemstream : virtual membuf, std::istream {
    imemstream(char const *base, size_t size)
      : membuf(base, size), std::istream(static_cast<std::streambuf *>(this)) {
    }
  };

  imemstream inStream(modelSrc, modelLen);
//  string inputfile = "models/teapot.obj";
//  string inputfile = "models/teapot_low.obj";
  string error;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, &inStream, nullptr, true)) {
    cerr << error << endl;
    exit(1);
  }

  if (!error.empty()) {
    cerr << error << endl;
  }

  processTeapot(attrib, shapes);
}

void processTeapot(const tinyobj::attrib_t &attrib, const vector<tinyobj::shape_t> &shapes) {
  random_device rd;
  mt19937 rng(rd());
  uniform_real_distribution<float> dist(0, 1);

  teapotVerts.resize(attrib.vertices.size() / 3);
  for (size_t i = 0, end = attrib.vertices.size() / 3; i < end; i++) {
    teapotVerts[i].pos = {
      attrib.vertices[3 * i + 0],
      attrib.vertices[3 * i + 1],
      attrib.vertices[3 * i + 2],
      1
    };
    teapotVerts[i].color = {
      1, 1, 1, 1
    };
    teapotVerts[i].normal = {
      attrib.normals[3 * i + 0],
      attrib.normals[3 * i + 1],
      attrib.normals[3 * i + 2]
    };
    teapotVerts[i].uv = {
      attrib.texcoords[3 * i + 0],
      attrib.texcoords[3 * i + 1]
    };
  }

  for (auto shape_iter = shapes.cbegin(), end = shapes.cend(); shape_iter < end; shape_iter++) {
    auto shape = *shape_iter;
    auto offset = teapotIndicies.size();
    teapotIndicies.resize(teapotIndicies.size() + shape.mesh.indices.size());

    for (size_t i = 0, end = shape.mesh.indices.size(); i < end; i++) {
      auto index = shape.mesh.indices[i];
      teapotIndicies[offset + i] = index.vertex_index;
    }
  }

  cout << "Loaded " << teapotVerts.size() << " verts, " << teapotIndicies.size() << " indicies" << endl;
}

const Framebuffer<f32_color_t> *getFB() {
  return renderer->getFramebuffer();
}

size_t getFBWidth() {
  return renderer->getWidth();
}

size_t getFBHeight() {
  return renderer->getHeight();
}

};
