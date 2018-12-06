#-------------------------------------------------
#
# Project created by QtCreator 2016-08-08T12:24:53
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = system-monitor
TEMPLATE = app

target.path = /usr/bin

desktop.path = /usr/share/applications
desktop.files += system-monitor.desktop

INSTALLS += target desktop

#icon.path = utilities-system-monitor
#icon.files += system-monitor.png

VERSION_MAJOR = 0
VERSION_MINOR = 0
VERSION_BUILD = 121

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

SOURCES += main.cpp\
        mainwindow.cpp \
    processinformationworker.cpp \
    workerthread.cpp \
    processtools.cpp \
    resourcesworker.cpp

HEADERS  += mainwindow.h \
    processinformationworker.h \
    tablenumberitem.h \
    workerthread.h \
    processtools.h \
    hashqstring.h \
    resourcesworker.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++14 -Wall
LIBS += -L"libprocps6" -lprocps -lstdc++fs

DISTFILES += \
    system-monitor.desktop
