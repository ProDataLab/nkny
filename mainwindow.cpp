#include "mainwindow.h"
#include "pairtabpage.h"
#include "ibclient.h"
#include "ui_mainwindow.h"
#include "ui_pairtabpage.h"
#include "ui_contractdetailswidget.h"
#include "contractdetailswidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTableWidget* tab = ui->homeTableWidget;

    QStringList hlables;
    hlables << "Pair1"
            << "Pair2"
            << "Price1"
            << "Price2"
            << "Ratio"
            << "Delta/StdDev"
            << "Percent From Mean"
            << "Correlation"
            << "Cointegration"
            << "Volatility"
            << "RSI of Ratio"
            << "RSI of Spread";
    tab->setColumnCount(hlables.size());
    tab->setHorizontalHeaderLabels(hlables);

    tab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
//    tab->verticalHeader()->hide();
    tab->setShowGrid(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    qDeleteAll(m_pairTabPages);
}

void MainWindow::on_action_New_triggered()
{
    PairTabPage* page = new PairTabPage(m_ibClient, this);

    m_pairTabPages.append(page);

    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    QString sym1 = page->getUi()->pair1ContractDetailsWidget->getUi()->symbolLineEdit->text();
    QString sym2 = page->getUi()->pair2ContractDetailsWidget->getUi()->symbolLineEdit->text();

    ui->tabWidget->addTab(page, sym1 + "/" + sym2);
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
    ui->actionConnect_To_TWS->setText("TWS Connected");

    m_ibClient->connectToTWS("127.0.0.1", 7496, 0);

    ui->action_New->setEnabled(true);

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
    m_ibClient->setOrderId(orderId);
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




