#include "mainwindow.h"
#include "pairtabpage.h"
#include "ibclient.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_New_triggered()
{
    QString cntStr(QString::number(ui->tabWidget->count()));
    ui->tabWidget->addTab(new PairTabPage(this), cntStr);
}

void MainWindow::on_actionConnect_To_TWS_triggered()
{
    // TODO: set button text and color when connected
    m_ibClient = new IBClient(this);

    m_ibClient->connectToTWS("127.0.0.1", 7496, 0);
}
