#include "mainwindow.h"
#include "pairtabpage.h"
#include "ibclient.h"
#include "ui_mainwindow.h"
#include "ui_pairtabpage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
    qDeleteAll(m_pairTabPages);
}

void MainWindow::on_action_New_triggered()
{
    QString cntStr(QString::number(ui->tabWidget->count()));
    PairTabPage* page = new PairTabPage(m_ibClient, this);

    connect(m_ibClient, SIGNAL(historicalData(long,QByteArray,double,double,double,double,int,int,double,int)),
            page, SLOT(onHistoricalData(long,QByteArray,double,double,double,double,int,int,double,int)));
    connect(m_ibClient, SIGNAL(realtimeBar(long,long,double,double,double,double,long,double,int)),
            page, SLOT(onRealTimeBars(long,long,double,double,double,double,long,double,int)));
    connect(m_ibClient, SIGNAL(tickGeneric(long,TickType,double)),
            page, SLOT(onTickGeneric(long,TickType,double)));
    connect(m_ibClient, SIGNAL(tickPrice(long,TickType,double,int)),
            page, SLOT(onTickPrice(long,TickType,double,int)));
    connect(m_ibClient, SIGNAL(tickSize(long,TickType,int)),
            page, SLOT(onTickSize(long,TickType,int)));

    m_pairTabPages.append(page);
    ui->tabWidget->addTab(page, cntStr);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::on_actionConnect_To_TWS_triggered()
{
    // TODO: set button text and color when connected
    m_ibClient = new IBClient(this);

    connect(m_ibClient, SIGNAL(managedAccounts(QByteArray)),
            this, SLOT(onManagedAccounts(QByteArray)));
    connect(m_ibClient, SIGNAL(error(int,int,QByteArray)),
            this, SLOT(onIbError(int,int,QByteArray)));
    connect(m_ibClient, SIGNAL(nextValidId(long)),
            this, SLOT(onNextValidId(long)));
    connect(m_ibClient, SIGNAL(currentTime(long)),
            this, SLOT(onCurrentTime(long)));

    connect(m_ibClient, SIGNAL(twsConnected()),
            this, SLOT(onTwsConnected()));
    connect(m_ibClient, SIGNAL(connectionClosed()),
            this, SLOT(onTwsConnectionClosed()));

    ui->actionConnect_To_TWS->setEnabled(false);

    m_ibClient->connectToTWS("127.0.0.1", 7496, 0);

}

void MainWindow::onManagedAccounts(const QByteArray &msg)
{
    qDebug() << "ManagedAccounts:" << msg;
}

void MainWindow::onIbError(const int id, const int errorCode, const QByteArray errorString)
{
    qDebug() << "IbError:" << id << errorCode << errorString;
}

void MainWindow::onNextValidId(long orderId)
{
    qDebug() << "NextValidId:" << orderId;
}

void MainWindow::onCurrentTime(long time)
{
    QDateTime dt(QDateTime::fromTime_t(time));

    qDebug() << "CurrentTime:" << dt;
}

void MainWindow::onTwsConnected()
{
    if (ui->actionConnect_To_TWS->isEnabled())
        ui->actionConnect_To_TWS->setEnabled(false);
}

void MainWindow::onTwsConnectionClosed()
{
    ui->actionConnect_To_TWS->setEnabled(true);
}
