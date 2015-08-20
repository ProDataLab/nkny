#-------------------------------------------------
#
# Project created by QtCreator 2015-08-06T17:59:42
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = nkny
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    pairtabpage.cpp \
    ibclient.cpp \
    security.cpp


HEADERS  += mainwindow.h \
    qcustomplot.h \
    pairtabpage.h \
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
    security.h


FORMS    += mainwindow.ui \
    pairtabpage.ui


INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

DISTFILES += \
    README.txt


