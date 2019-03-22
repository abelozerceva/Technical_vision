cmake_minimum_required(VERSION 2.8)

project(markers)
add_executable(${PROJECT_NAME} main.cpp libs/detect_markers.cpp)
find_package( OpenCV REQUIRED )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )

#add_executable(${PROJECT_NAME} ${SRC_LIST} "samples.hpp")

#set(SOURSES detect_markers.cpp)
#set(HEADERS samples.hpp)
#add_library(lib ${SOURCES} ${HEADERS})
#add_executable(libs samples.hpp)
#target_include_directories(libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
