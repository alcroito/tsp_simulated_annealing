#-------------------------------------------------
#
# Project created by QtCreator 2015-02-18T12:04:28
#
#-------------------------------------------------

QT       += core

QT       -= gui core

TARGET = tsp_simulated_annealing
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app


SOURCES += main.cpp

OTHER_FILES += \
    data.txt

install_it.path = $$OUT_PWD
install_it.files = data.txt

INSTALLS += install_it
