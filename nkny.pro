#-------------------------------------------------
#
# Project created by QtCreator 2015-08-06T17:59:42
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = nkny
TEMPLATE = app

QMAKE_CXXFLAGS_DEBUG += -Werror

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    ibclient.cpp \
    security.cpp \
    helpers.cpp \
    pairtabpage.cpp \
    stddevlayertab.cpp \
    contractdetailswidget.cpp


HEADERS  += mainwindow.h \
    qcustomplot.h \
    ibclient.h \
    ibdefines.h \
    iborder.h \
    ibtagvalue.h \
    ibcontract.h \
    ibticktype.h \
    ibfadatatype.h \
    iborderstate.h \
    ibexecution.h \
    ibbardata.h \
    ibscandata.h \
    ibcommissionreport.h \
    ibsocketerrors.h \
    helpers.h \
    security.h \
    pairtabpage.h \
    adfhelper.h \
    stddevlayertab.h \
    contractdetailswidget.h \
    JohansenCointegration/CommonTypes.h


FORMS    += mainwindow.ui \
    pairtabpage.ui \
    stddevlayertab.ui \
    contractdetailswidget.ui


INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

DISTFILES += \
    README.txt




#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/release/ -lnewmat-mybuild
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/debug/ -lnewmat-mybuild

#INCLUDEPATH += $$PWD/../newmat_10
#DEPENDPATH += $$PWD/../newmat_10

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/release/libnewmat-mybuild.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/debug/libnewmat-mybuild.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/release/newmat-mybuild.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-newmat-mybuild-Desktop_Qt_5_5_0_MinGW_32bit-Debug/debug/newmat-mybuild.lib
