#-------------------------------------------------
#
# Project created by QtCreator 2014-11-28T00:19:41
#
#-------------------------------------------------

CONFIG -= qt
TEMPLATE = lib
TARGET = model
CONFIG += staticlib

DEFINES += ATTRIBUTE_EXPORTS


win32 { DEFINES += WIN32 WIN64}


CONFIG(debug, debug|release) { DEFINES += _DEBUG }
CONFIG(release, debug|release) { DEFINES += NDEBUG }


INCLUDEPATH += . ..


SOURCES += \
    IO_obj.cpp \
    IO_off.cpp \
    IO_poly.cpp \
    IO_stl.cpp \
    IO.cpp \
    point_cloud.cpp \
    surface_mesh.cpp

HEADERS += \
    IO.h \
    point_cloud.h \
    properties.h \
    surface_mesh.h \
    types.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

#Specify which sdk to use in your .pro file as follows:
#macx {
#    QMAKE_MAC_SDK = macosx10.9
#}
