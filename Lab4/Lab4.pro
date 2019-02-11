TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/local/include
LIBS +=  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_tracking -lopencv_imgproc -lopencv_objdetect

SOURCES += main.cpp
