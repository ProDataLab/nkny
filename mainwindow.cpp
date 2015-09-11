#include "mainwindow.h"
#include "pairtabpage.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "iborder.h"
#include "iborderstate.h"
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
    , m_ibClient(NULL)
{
    ui->setupUi(this);

    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->setVisible(false);

    QTableWidget* tab = ui->homeTableWidget;

    m_headerLabels << "Pair1"
            << "Pair2"
            << "TimeFrame"
            << "Price1"
            << "Price2"
            << "Ratio"
            << "RatioMA"
            << "StdDev"
            << "PcntFromMA"
            << "Corr"
            << "Vola"
            << "RSI";

    tab->setColumnCount(m_headerLabels.size());
    tab->setHorizontalHeaderLabels(m_headerLabels);

    tab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
//    tab->verticalHeader()->hide();
    tab->setShowGrid(true);


    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(onTabCloseRequested(int)));

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
//    qDeleteAll(m_pairTabPages);
}

void MainWindow::on_action_New_triggered()
{
    PairTabPage* page = new PairTabPage(m_ibClient, m_managedAccounts, this);

    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    QString sym1 = page->getUi()->pair1ContractDetailsWidget->getUi()->symbolLineEdit->text();
    QString sym2 = page->getUi()->pair2ContractDetailsWidget->getUi()->symbolLineEdit->text();

    ui->tabWidget->addTab(page, sym1 + "/" + sym2);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    m_pairTabPageMap[ui->tabWidget->currentIndex()] = page;

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

    connect(m_ibClient, SIGNAL(orderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)),
            this, SLOT(onOrderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)));
    connect(m_ibClient, SIGNAL(openOrder(long,Contract,Order,OrderState)),
            this, SLOT(onOpenOrder(long,Contract,Order,OrderState)));

    ui->actionConnect_To_TWS->setEnabled(false);
    ui->actionConnect_To_TWS->setText("TWS Connected");

    m_ibClient->connectToTWS("127.0.0.1", 7496, 0);

    ui->action_New->setEnabled(true);

    QSettings settings;

    int numTabPages = settings.value("numPairTabPages", 0).toInt();

    for (int i=0;i<numTabPages;++i) {
        PairTabPage* p = new PairTabPage(m_ibClient, m_managedAccounts, this);
        p->readSettings();

        QString tabSymbol = p->getTabSymbol();

        ui->tabWidget->addTab(p, tabSymbol);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        m_pairTabPageMap[ui->tabWidget->currentIndex()] = p;
    }

}

void MainWindow::onManagedAccounts(const QByteArray &msg)
{
    m_managedAccounts = QString(msg).split(',', QString::SkipEmptyParts);
    qDebug() << "ManagedAccounts:" << m_managedAccounts;

    readPageSettings();
}

void MainWindow::onIbError(const int id, const int errorCode, const QByteArray errorString)
{
    qDebug() << "IbError:" << id << errorCode << errorString;
}

void MainWindow::onNextValidId(long orderId)
{
    qDebug() << "[DEBUG-onNextValidId] orderId:" << orderId;
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

void MainWindow::onTabCloseRequested(int idx)
{
    qDebug() << "[DEBUG-onTabCloseRequested] 1";

    QString tabSymbol = ui->tabWidget->tabBar()->tabText(idx);

    PairTabPage* page = NULL;

    for (int i=0;i<m_pairTabPageMap.size();++i) {
        page = m_pairTabPageMap.values().at(i);
        if (page->getTabSymbol() == tabSymbol) {
            m_pairTabPageMap.remove(m_pairTabPageMap.keys().at(i));
            break;
        }
    }

    bool closeTab = page->reqClosePair();

    qDebug() << "[DEBUG-onTabCloseRequested] 2";


    if (closeTab) {
        ui->tabWidget->removeTab(idx);
        delete page;
    }

    qDebug() << "[DEBUG-onTabCloseRequested] 3";

    // else ... POP UP WINDOW THAT THERE ARE ORDERS ACTIVE
}

void MainWindow::onOrderStatus(long orderId, const QByteArray &status, int filled, int remaining, double avgFillPrice,
                                int permId, int parentId, double lastFillPrice, int clientId, const QByteArray &whyHeld)
{
    if (filled > 0) {

    }
    qDebug() << "[DEBUG-onOrderStatus]"
             << orderId
             << status
             << filled
             << remaining
             << avgFillPrice
             << permId
             << parentId
             << lastFillPrice
             << clientId
             << whyHeld;
}

void MainWindow::onOpenOrder(long orderId, const Contract &contract, const Order &order, const OrderState &orderState)
{
    qDebug() << "[DEBUG-onOpenOrder]"
             << orderId
             << contract.symbol
             << order.action
             << orderState.initMargin
             << orderState.maintMargin
             << orderState.status;
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("state", saveState());
    settings.setValue("numPairTabPages", ui->tabWidget->count());
    settings.endGroup();

    settings.beginWriteArray("pages");
    for (int i=0;i< ui->tabWidget->count();++i) {
        PairTabPage* page = qobject_cast<PairTabPage*>(ui->tabWidget->widget(i));
        settings.setValue("tabSymbol", page->getTabSymbol());
        page->writeSettings();
    }
    settings.endArray();
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    QByteArray state = settings.value("state", QByteArray())
                                                  .toByteArray();
   restoreState(state);
   resize(size);
   move(pos);



}

void MainWindow::readPageSettings()
{
    QSettings s;

    int size = s.beginReadArray("pages");
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        PairTabPage* p = new PairTabPage(m_ibClient, m_managedAccounts, this);
        p->setTabSymbol(s.value("tabSymbol").toString());
        p->readSettings();

        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

        ui->tabWidget->addTab(p, p->getTabSymbol());
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        m_pairTabPageMap[ui->tabWidget->currentIndex()] = p;
    }
}
QStringList MainWindow::getHeaderLabels() const
{
    return m_headerLabels;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}





