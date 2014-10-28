QT           += xml svg core gui network

HEADERS       = window.h \
                generic.h \
    gateway.h \
    infopacket.h \
    modem.h \
    monitor.h
SOURCES       = main.cpp \
                window.cpp \
    gateway.cpp \
    infopacket.cpp \
    modem.cpp \
    monitor.cpp
RESOURCES     = datahub.qrc

include(../libraries/qserialdevice/qserialdevice/src/qserialdeviceenumerator/qserialdeviceenumerator.pri)
include(../libraries/qserialdeviceinfo/qserialdeviceinfo.pri)
include(../libraries/qserialdevice/qserialdevice/src/qserialdevice/qserialdevice.pri)

VERSION = 1.5

TARGET = DataHub
TEMPLATE = app

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS datahub.pro resources images
INSTALLS += sources


win32 {
LIBS             += -lsetupapi -lAdvapi32 -lUser32
}

RC_FILE = datahub.rc
OTHER_FILES += datahub.rc \
    datahub.rc

wince* {
	CONFIG(debug, release|debug) {
		addPlugins.sources = $$QT_BUILD_TREE/plugins/imageformats/qsvgd4.dll
	}
	CONFIG(release, release|debug) {
		addPlugins.sources = $$QT_BUILD_TREE/plugins/imageformats/qsvg4.dll
	}
	addPlugins.path = imageformats
	DEPLOYMENT += addPlugins
}
