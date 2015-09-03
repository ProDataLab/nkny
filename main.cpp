#include "mainwindow.h"
#include <QApplication>
#include <QRect>
#include <QMargins>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.showMaximized();

    QRect rec = QApplication::desktop()->screenGeometry();
    QMargins mar(100,100,100,100);
    rec -= mar;

    w.setGeometry(rec);

    w.show();
    return a.exec();
}
