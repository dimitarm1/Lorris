TEMPLATE = subdirs
CONFIG += ordered
win32 {
    DEFINES += Q_OS_WIN32
}

SUBDIRS = dep \
          src
src.depends = dep

CONFIG(debug, debug|release):DESTDIR = "$$PWD/bin/debug"
else:DESTDIR = "$$PWD/bin/release"

TARGET = "Lorris"
QMAKE_CXXFLAGS += -std=c++0x
