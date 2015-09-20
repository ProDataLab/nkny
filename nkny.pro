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
    orderstablewidget.cpp \
    datatoolboxwidget.cpp \
    smtp/emailaddress.cpp \
    smtp/mimeattachment.cpp \
    smtp/mimecontentformatter.cpp \
    smtp/mimefile.cpp \
    smtp/mimehtml.cpp \
    smtp/mimeinlinefile.cpp \
    smtp/mimemessage.cpp \
    smtp/mimemultipart.cpp \
    smtp/mimepart.cpp \
    smtp/mimetext.cpp \
    smtp/quotedprintable.cpp \
    smtp/smtpclient.cpp

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
    orderstablewidget.h \
    datatoolboxwidget.h \
    smtp/emailaddress.h \
    smtp/mimeattachment.h \
    smtp/mimecontentformatter.h \
    smtp/mimefile.h \
    smtp/mimehtml.h \
    smtp/mimeinlinefile.h \
    smtp/mimemessage.h \
    smtp/mimemultipart.h \
    smtp/mimepart.h \
    smtp/mimetext.h \
    smtp/quotedprintable.h \
    smtp/smtpclient.h \
    smtp/smtpexports.h


FORMS    += mainwindow.ui \
    pairtabpage.ui \
    stddevlayertab.ui \
    contractdetailswidget.ui \
    globalconfigdialog.ui \
    datatoolboxwidget.ui


INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

DISTFILES += \
    README.rst


