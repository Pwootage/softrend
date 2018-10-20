cmake_minimum_required(VERSION 3.10)
project(softrend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mavx2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mavx2")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=thin -mavx2")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=thin -mavx2")

find_package(glfw3 3.2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

set(SOURCE_FILES
        SoftwareRasterizer.cpp
        main.cpp)

set(HEADER_FILES
        SoftwareRasterizer.hpp)

add_executable(softrend ${SOURCE_FILES} ${HEADER_FILES} ${GLM_INCLUDE_DIR})

target_link_libraries(softrend glfw ${OPENGL_gl_LIBRARY})