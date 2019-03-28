cmake_minimum_required(VERSION 3.13)
project(markers)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME} main.cpp libs/detect_markers.cpp libs/draw_cube.cpp)
find_package( OpenCV REQUIRED )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )