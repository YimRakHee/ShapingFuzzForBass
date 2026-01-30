cmake_minimum_required(VERSION 3.15)
project(ShapingFuzzForBass VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3 -march=native -fPIC)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ShapingFuzzForBass"
)

if(NOT DEFINED LV2_INSTALL_DIR)
    set(LV2_INSTALL_DIR "$ENV{HOME}/.lv2/${PROJECT_NAME}.lv2")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${LV2_INSTALL_DIR})
install(FILES manifest.ttl plugin.ttl DESTINATION ${LV2_INSTALL_DIR})

message(STATUS "Install destination: ${LV2_INSTALL_DIR}")
