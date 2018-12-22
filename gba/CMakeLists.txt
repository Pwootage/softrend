
include_directories(/opt/devkitpro/libgba/include)
include_directories(build)

set(GBA_SRCS
        src-gba/GBAMode3Framebuffer.hpp
        src-gba/GBAMode3Framebuffer.cpp
        src-gba/main.cpp)

add_library(gba ${GBA_SRCS})
