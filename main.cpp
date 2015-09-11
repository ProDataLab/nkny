#include "mainwindow.h"
#include <QApplication>
#include <QRect>
#include <QMargins>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("prodatalab");
    a.setOrganizationDomain("prodatalab.com");
    a.setApplicationName("nkny");
    MainWindow w;

    w.show();
    return a.exec();
}
