include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(windeployqt))

option(host_build)
QT = core-private
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_FOREACH

include(../shared/winutils/winutils.pri)
SOURCES += main.cpp

CONFIG += force_bootstrap

win32: LIBS += -lshlwapi

QMAKE_TARGET_DESCRIPTION = "Qt Windows Deployment Tool"

qtConfig(relocatable): DEFINES += QT_RELOCATABLE

load(qt_tool)
