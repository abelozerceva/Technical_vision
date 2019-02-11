TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QT += gui

INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules
LIBS +=  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_tracking -lopencv_imgproc -lopencv_objdetect

INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules/core/include
INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules/imgcodecs/include
INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules/videoio/include
INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules/imgproc/include
INCLUDEPATH += /home/anastasia/opencv_build/opencv/modules/objdetect/include

SOURCES += \
        main.cpp
