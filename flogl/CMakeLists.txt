# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (FastLED-OpenCV-Lib)

find_package(OpenCV REQUIRED)

include_directories(
   .
   ${CMAKE_CURRENT_BINARY_DIR}
   "/usr/include/cairo"
)

set(ALL_LIBS
   ${OpenCV_LIBS}
   X11
   Xfixes
   Xcomposite
   cairo
)

set(FASTLED_SOURCES
   "${FastLED_SRC}/colorutils.cpp"
   "${FastLED_SRC}/colorpalettes.cpp"
   "${FastLED_SRC}/hsv2rgb.cpp"
   "${FastLED_SRC}/lib8tion.cpp"
   # "${FastLED_SRC}/FX.cpp"
   # "${FastLED_SRC}/FX_fcn.cpp"   
   )

set_source_files_properties(${FASTLED_SOURCES}
   PROPERTIES COMPILE_FLAGS "-include platform.h -Wno-mismatched-tags -Wno-unused-private-field"
   )
source_group(FastLED FILES ${FASTLED_SOURCES})

set(FLOGL_SOURCES
   flogl.cpp
   #flogl_x11.cpp
   "${CMAKE_CURRENT_BINARY_DIR}/LedTexture.hpp"
   )

# Disable annoying #pragma messages.
set_source_files_properties(${FLOGL_SOURCES} PROPERTIES COMPILE_FLAGS " -DFASTLED_INTERNAL")

add_library(flogl
   ${FLOGL_SOURCES}
   ${FASTLED_SOURCES}
   )

target_link_libraries(flogl
   ${ALL_LIBS}
   )
