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
    contractdetailswidget.cpp \
    mdiarea.cpp \
    globalconfigdialog.cpp \
    datatoolboxwidget.cpp \
    smtp.cpp \
    orderstablewidget.cpp \
    portfoliotablewidget.cpp \
    welcomedialog.cpp

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
    mdiarea.h \
    globalconfigdialog.h \
    datatoolboxwidget.h \
    smtp.h \
    orderstablewidget.h \
    portfoliotablewidget.h \
    welcomedialog.h



FORMS    += mainwindow.ui \
    pairtabpage.ui \
    stddevlayertab.ui \
    contractdetailswidget.ui \
    globalconfigdialog.ui \
    datatoolboxwidget.ui \
    welcomedialog.ui


INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

DISTFILES += \
    README.rst


