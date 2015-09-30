#include "mainwindow.h"
#include <QApplication>
#include <QRect>
#include <QMargins>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("prodatalab");
    QCoreApplication::setOrganizationDomain("prodatalab.com");
    QCoreApplication::setApplicationName("nkny");
    MainWindow w;

//    QSettings s;
//    s.clear();

    w.show();
    return a.exec();
}
