
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

DESTDIR = bin
OBJECTS_DIR = obj


