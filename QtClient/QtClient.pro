QT += core network gui widgets

TARGET = QtClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += debug_and_release

SOURCES += \
    client.cpp \
    chatwindow.cpp \
    main.cpp

FORMS += \
    chatwindow.ui

HEADERS += \
    chatwindow.h \
    client.h
