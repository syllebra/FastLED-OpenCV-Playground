cmake_minimum_required (VERSION 3.0)
project (FastLED-OpenGL)

set(FASTLED_DIR "/home/cedric/PERSO/FastLED-OpenCV-Playground/FastLED" CACHE PATH "Path to FastLED installation")

if(FASTLED_DIR STREQUAL "")
   message("FASTLED_DIR not set, searching default locations")
endif()

set(WS2812FX_DIR "/home/cedric/PERSO/FastLED-OpenCV-Playground/WS2812FX" CACHE PATH "Path to WS2812FX installation")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/flogl/cmake/Modules")
find_package(FastLED REQUIRED)


include_directories(
   ${GLFW_INCLUDE_DIR}
   ${GLM_INCLUDE_DIR}
   flogl
   .
   )

add_definitions(
   -std=c++11
   -DGLFW_INCLUDE_GLCOREARB=1
   -DGLFW_INCLUDE_GLEXT=1
   -Werror
   -Wall
   )

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
   add_definitions(
      # the following two are warnings about deprecations
      # on mac OS. Potentially some work required in the future...
      -DGL_SILENCE_DEPRECATION=1
      -Wno-deprecated-register
      )
endif()

add_subdirectory(flogl)


add_executable(ledsplayground examples/ledsplayground.cpp)
# Disable annoying #pragma messages.
set_source_files_properties(examples/ledsplayground.cpp PROPERTIES COMPILE_FLAGS " -DFASTLED_INTERNAL")

target_link_libraries(ledsplayground flogl)
