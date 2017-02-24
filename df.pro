
QT       += core

QT       -= gui

TARGET = df 
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += link_pkgconfig

PKGCONFIG += glfw3

TEMPLATE = app

SOURCES += \
   main.cpp

CONFIG += c++11

INCLUDEPATH += ./ ./deps/glad/include ./deps/stb
LIBS = -lGL -lGLU -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -fopenmp

DESTDIR = bin
OBJECTS_DIR = obj

gcc:QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -fopenmp -D_GLIBCXX_PARALLEL
gcc:QMAKE_CXXFLAGS+= -fopenmp -D_GLIBCXX_PARALLEL
