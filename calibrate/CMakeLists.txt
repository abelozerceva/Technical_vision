cmake_minimum_required(VERSION 2.8)

project(calibrate)
add_executable(${PROJECT_NAME} "main.cpp")
find_package( OpenCV REQUIRED )
#add_executable( DisplayImage DisplayImage.cpp )
target_link_libraries( calibrate ${OpenCV_LIBS} )
