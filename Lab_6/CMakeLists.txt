cmake_minimum_required(VERSION 3.13)
project(Lab_6)

set(CMAKE_CXX_STANDARD 14)

add_executable(Lab_6 main.cpp)
find_package( OpenCV REQUIRED )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )