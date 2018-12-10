cmake_minimum_required(VERSION 3.10)
project(softrend)

# Detect emscritpen, I guess
if (CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
    set(EMSCRIPTEN 1)
endif ()

set(CMAKE_CXX_STANDARD 17)

# C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17") # for whatever reason emmake breaks the regular std definition
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -O3")

# ARM-specific flags

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mfpu=neon -mfloat-abi=hard")
endif ()

# Emscripten-specific flags
if (EMSCRIPTEN)
    set(EMSCRIPTEN_FLAGS "-s LEGACY_GL_EMULATION=1 -s GL_FFP_ONLY=1 -s USE_GLFW=3 -s TOTAL_MEMORY=128MB")
    set(EMSCRIPTEN_LINK_FLAGS "--preload-file models/teapot-low.obj --preload-file models/teapot.obj")

    set(WASM "1" CACHE TYPE BOOL)
    if (WASM)
        message("Compiling to WebAssembly")
        set(EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS} -s WASM=1")
    else ()
        message("Compiling without WebAssembly and with SIMD")
        set(EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS} -s SIMD=1 -s WASM=0")
    endif ()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_LINK_FLAGS}")
endif ()

# Deps
if (NOT EMSCRIPTEN)
    find_package(glfw3 3.2 REQUIRED)
    find_package(OpenGL REQUIRED)
endif ()

# Files
include_directories(include)

set(SOURCE_FILES
        main.cpp
        src/shader/BasicVertexShader.cpp
        src/shader/PhongFragmentShader.cpp)

set(HEADER_FILES
        src/SoftwareRasterizer.hpp
        src/shader/VertexShader.hpp
        src/shader/VertexTypes.hpp
        src/shader/BasicVertexShader.hpp
        src/shader/FragmentShader.hpp
        src/shader/PhongFragmentShader.hpp
        src/buffers/VertexBuffer.hpp
        src/buffers/IndexBuffer.hpp
        )

set(RESOURCE_FILES
        models/teapot.obj
        models/teapot-low.obj
        )

# copy our resources
file(COPY ${RESOURCE_FILES}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models/)

# Set up emscripten or regular executable appropriately
if (EMSCRIPTEN)
    add_executable(softrend.html ${SOURCE_FILES} ${HEADER_FILES} ${GLM_INCLUDE_DIR})

    target_link_libraries(softrend.html glm)


    #    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
    #    separate_arguments(EMSCRIPTEN_FLAGS_SEP UNIX_COMMAND "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
    #    #    message("Flags: ${EMSCRIPTEN_FLAGS_SEP}")
    #    add_custom_command(
    #            COMMAND ${CMAKE_CXX_COMPILER} ARGS ${EMSCRIPTEN_FLAGS_SEP} "${CMAKE_CURRENT_BINARY_DIR}/softrend.bc" -o "${CMAKE_CURRENT_BINARY_DIR}/softrend.js"
    #            OUTPUT softrend.js
    #            COMMENT "Compile/optimize to js..."
    #            VERBATIM
    #    )
    #
    #    add_custom_target(
    #            softrend_js ALL
    #            DEPENDS softrend.js
    #    )
    #
    #    add_dependencies(softrend_js softrend.bc)

else ()
    add_executable(softrend ${SOURCE_FILES} ${HEADER_FILES} ${GLM_INCLUDE_DIR})

    target_link_libraries(softrend glfw ${OPENGL_gl_LIBRARY})
endif ()
