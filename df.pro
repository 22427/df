
QT       += core

QT       -= gui

TARGET = df 
CONFIG   += console
CONFIG   -= app_bundle


TEMPLATE = app

SOURCES += \
   src/main.cpp

CONFIG += c++11

INCLUDEPATH += ./ ./deps/stb
QMAKE_LFLAGS += -fopenmp
QMAKE_CXXFLAGS += -fopenmp

DESTDIR = bin
OBJECTS_DIR = obj

DISTFILES += \
    test_font.txt

HEADERS += \
    src/bin_img.h \
    src/shape.h \
    src/image2d.h


