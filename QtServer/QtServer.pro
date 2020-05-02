QT += core network gui widgets

TARGET = QtServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += debug_and_release

HEADERS += \
    server.h \
    serverwindow.h \
    worker.h

SOURCES += \
    main.cpp \
    server.cpp \
    serverwindow.cpp \
    worker.cpp

FORMS += \
    serverwindow.ui
