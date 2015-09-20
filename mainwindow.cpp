#include "mainwindow.h"
#include "pairtabpage.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "iborder.h"
#include "iborderstate.h"
#include "ui_mainwindow.h"
#include "ui_pairtabpage.h"
#include "ui_contractdetailswidget.h"
#include "ui_globalconfigdialog.h"
#include "contractdetailswidget.h"
#include "globalconfigdialog.h"
#include "security.h"
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
    ui->tabWidget->tabBar()->tabButton(1, QTabBar::RightSide)->setVisible(false);

    m_orderHeaderLabels << "Pair"
                      << "Sym1"
                      << "Sym2"
                      << "Size1"
                      << "Size2"
                      << "Cost/Unit1"
                      << "Cost/Unit2"
                      << "TotalCost1"
                      << "TotalCost2"
                      << "Last1"
                      << "Last2"
                      << "Diff1"
                      << "Diff2"
                      << "PercentChange1"
                      << "PercentChange2";

    QTableWidget* tab = ui->ordersTableWidget;
    tab->setEnabled(true);
    tab->setVisible(false);
    tab->setColumnCount(m_orderHeaderLabels.size());
    tab->setHorizontalHeaderLabels(m_orderHeaderLabels);
    tab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab->setShowGrid(true);

    tab = ui->homeTableWidget;

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
            << "RSI"
            << "RSISpread";

    tab->setColumnCount(m_headerLabels.size());
    tab->setHorizontalHeaderLabels(m_headerLabels);

    tab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
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
    page->setTabSymbol(sym1 + "/" + sym2);
    ui->tabWidget->addTab(page, sym1 + "/" + sym2);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    m_pairTabPageMap[ui->tabWidget->currentIndex()] = page;

    page->getUi()->pair1UnitOverrideLabel->setText(sym1 + QString(" Units"));
    page->getUi()->pair2UnitOverrideLabel->setText(sym2 + QString(" Units"));
}

void MainWindow::on_actionConnect_To_TWS_triggered()
{

    m_ibClient = new IBClient(this);


    connect(m_ibClient, SIGNAL(managedAccounts(QByteArray)),
            this, SLOT(onManagedAccounts(QByteArray)));
    connect(m_ibClient, SIGNAL(error(int,int,QByteArray)),
            this, SLOT(onIbError(int,int,QByteArray)));
    connect(m_ibClient, &IBClient::ibSocketError,
            this, &MainWindow::onIbSocketError);
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

//    QSettings s;
//    s.beginGroup("mainwindow");
//    int clientId = s.value("clientId", 0).toInt() + 1;
    int clientId = 0;
    m_ibClient->connectToTWS("127.0.0.1", 7496, clientId);
//    s.setValue("clientId", clientId);
//    s.endGroup();

//    QProgressDialog progress(this);
//    progress.setWindowModality(Qt::WindowModal);
//    progress.setLabelText("working...");
//    progress.setCancelButton(0);
//    progress.setRange(0,0);
//    progress.setMinimumDuration(0);
//    progress.show();
//    //do what u want...
//    progress.cancel();
}

void MainWindow::onManagedAccounts(const QByteArray &msg)
{
    m_managedAccounts = QString(msg).split(',', QString::SkipEmptyParts);
    qDebug() << "ManagedAccounts:" << m_managedAccounts;
    ui->actionGlobal_Config->setEnabled(true);
    m_globalConfigDialog.setMangagedAccounts(m_managedAccounts);

    readPageSettings();
}

void MainWindow::onIbError(const int id, const int errorCode, const QByteArray errorString)
{
    qDebug() << "IbError:" << id << errorCode << errorString;
}

void MainWindow::onIbSocketError(const QString &error)
{
    static bool enacted = false;
    if (!enacted && error == QString("Connection Refused")) {
        QMessageBox msgBox;
        msgBox.setText("Connection Refused");
        msgBox.setInformativeText("Please ensure TWS is running and accepts API calls.");
//        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//        msgBox.setDefaultButton(QMessageBox::Save);
        enacted = true;
        /*int ret =*/ msgBox.exec();
        enacted = false;
    }

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
    ui->actionConnect_To_TWS->setEnabled(false);
    ui->actionConnect_To_TWS->setText("TWS Connected");
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

    if (!ui->ordersTab->isEnabled())
        ui->ordersTab->setEnabled(true);
    if (!ui->ordersTab->isVisible())
        ui->ordersTab->setVisible(true);

    int row = -1;
    int col = -1;
    for (int j=0;j<ui->ordersTableWidget->columnCount();++j) {
        if (ui->ordersTableWidget->horizontalHeaderItem(j)->text() == "Pair") {
            col = j;
            break;
        }
    }

    for (int i=0;i<m_pairTabPageMap.count();++i) {
        PairTabPage* p = m_pairTabPageMap.values().at(i);
        Security* s1 = p->getSecurities().at(0);
        Security* s2 = p->getSecurities().at(1);

        for (int j=0;j<ui->ordersTableWidget->rowCount();++j) {
            if (ui->ordersTableWidget->item(j, col)->text() == p->getTabSymbol()) {
                row = j;
                break;
            }
        }

        if (row ==  -1)
            ui->ordersTableWidget->setRowCount(ui->ordersTableWidget->rowCount()+1);

        for (int j=0;j<ui->ordersTableWidget->columnCount();++j) {
            if (row == -1) {
                row = ui->ordersTableWidget->rowCount() - 1;
                QTableWidgetItem* item = new QTableWidgetItem(QString(""));
                ui->ordersTableWidget->setItem(ui->ordersTableWidget->rowCount()-1, j, item);
            }
            QString headerItemText = ui->ordersTableWidget->horizontalHeaderItem(j)->text();
            QTableWidgetItem* item = ui->ordersTableWidget->item(row, j);

            bool isS1 = s1->getSecurityOrderMap().contains(orderId);
            double lastClose;

            if (headerItemText == "Pair")
                item->setText(p->getTabSymbol());
            else if (headerItemText == "Sym1") {
                if (p->getPair1ContractDetailsWidget()->getUi()->securityTypeComboBox->currentText() == QString("FUT"))
                    item->setText(s1->contract()->symbol.toUpper()
                                  + " [" + p->getPair1ContractDetailsWidget()->getUi()->expiryLineEdit->text() + "]");
                else
                    item->setText(s1->contract()->symbol.toUpper());
            }
            else if (headerItemText == "Sym2") {
                if (p->getPair2ContractDetailsWidget()->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
                    item->setText(s2->contract()->symbol.toUpper()
                                  + " [" + p->getPair2ContractDetailsWidget()->getUi()->expiryLineEdit->text() + "]");
                }
            }
                else {
                    item->setText(s2->contract()->symbol.toUpper());
            }
            if (isS1) {
                lastClose = s1->getHistData(p->getTimeFrame())->close.last();
                if (headerItemText == "Size1")
                    item->setText(QString::number(filled));
                else if (headerItemText == "Cost/Unit1")
                    item->setText(QString::number(avgFillPrice));
                else if (headerItemText == "Last1")
                    item->setText(QString::number(lastClose));
                else if (headerItemText == "Diff1")
                    item->setText(QString::number(lastClose - avgFillPrice));
                else if (headerItemText == "TotalCost1")
                    item->setText(QString::number(filled * avgFillPrice));
                else if (headerItemText == "PercentChange1")
                    item->setText(QString::number((lastClose-avgFillPrice) / avgFillPrice));
            }
            else {
                lastClose = s2->getHistData(p->getTimeFrame())->close.last();
                if (headerItemText == "Size2")
                    item->setText(QString::number(filled));
                else if (headerItemText == "Cost/Unit2")
                    item->setText(QString::number(avgFillPrice));
                else if (headerItemText == "Last2")
                    item->setText(QString::number(lastClose));
                else if (headerItemText == "Diff2")
                    item->setText(QString::number(lastClose - avgFillPrice));
                else if (headerItemText == "TotalCost2")
                    item->setText(QString::number(filled * avgFillPrice));
                else if (headerItemText == "PercentChange2")
                    item->setText(QString::number((lastClose-avgFillPrice) / avgFillPrice));
            }
        }

        for (int j=0;j<p->getSecurities().count();++j) {
            Security* s = p->getSecurities().at(j);
            if (!s->getSecurityOrderMap().contains(orderId))
                continue;
            SecurityOrder* so = s->getSecurityOrderMap()[orderId];
            so->status = status;
            so->filled = filled;
            so->remaining = remaining;
            so->avgFillPrice = avgFillPrice;
            so->permId = permId;
            so->parentId = parentId;
            so->lastFillPrice = lastFillPrice;
            so->clientId = clientId;
            so->whyHeld = whyHeld;
        }
    }
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

    for (int i=0;i<m_pairTabPageMap.count();++i) {
        PairTabPage* p = m_pairTabPageMap.values().at(i);
        for (int j=0;j<p->getSecurities().count();++j) {
            Security* s = p->getSecurities().at(j);
            if (!s->getSecurityOrderMap().contains(orderId))
                continue;
            SecurityOrder* so = s->getSecurityOrderMap()[orderId];
            so->order = order;
            so->orderState = orderState;
        }
        QString tabSymbol = p->getTabSymbol();
    }
}


void MainWindow::writeSettings()
{
    qDebug() << "[DEBUG-MainWindow::writeSettings]";

    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("state", saveState());
    settings.setValue("numPairTabPages", ui->tabWidget->count());
    settings.setValue("tabWidgetCurrentIndex", ui->tabWidget->currentIndex());
    settings.setValue("ordersTabEnabled", ui->ordersTab->isEnabled());

    settings.beginWriteArray("pages");

    int subtract = 0;
    for (int i=0;i< ui->tabWidget->count();++i) {
        if (ui->tabWidget->tabText(i) == QString("Home")){
            ++subtract;
            continue;
        }
        else if (ui->tabWidget->tabText(i) == QString("Orders")) {
            ++subtract;
            continue;
        }
        settings.setArrayIndex(i-subtract);
        PairTabPage* page = qobject_cast<PairTabPage*>(ui->tabWidget->widget(i));
        if (!page)
            continue;
        QString tabSymbol = page->getTabSymbol();
        qDebug() << "[DEBUG-MainWindow::writeSettings] tabSymbol" << i << ":" << tabSymbol;
        settings.setValue("tabSymbol", tabSymbol);
        page->writeSettings();
    }
    settings.endArray();
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("mainwindow");

    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();

    QSize size = settings.value("size", QSize(400, 400)).toSize();

    QByteArray state = settings.value("state", QByteArray())
                                                  .toByteArray();

    settings.endGroup();

   restoreState(state);
   resize(size);
   move(pos);



}

void MainWindow::readPageSettings()
{
    QSettings s;
    s.beginGroup("mainwindow");
    int size = s.beginReadArray("pages");

    qDebug() << "[DEBUG-readPageSettings] size" << size;

    for (int i=0;i<size;++i) {

        s.setArrayIndex(i);
        PairTabPage* p = new PairTabPage(m_ibClient, m_managedAccounts, this);
        QString tabSymbol = s.value("tabSymbol").toString();
        qDebug() << "[DEBUG-readPageSettings] tabSymbol" << i << ":" << tabSymbol;
        p->setTabSymbol(tabSymbol);
        p->readSettings();

        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

        ui->tabWidget->addTab(p, p->getTabSymbol());
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        m_pairTabPageMap[ui->tabWidget->currentIndex()] = p;
    }
    s.endArray();
    ui->tabWidget->setCurrentIndex(s.value("tabWidgetCurrentIndex").toInt());
    s.endGroup();
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


void MainWindow::on_actionGlobal_Config_triggered()
{
    m_globalConfigDialog.exec();
}
QStringList MainWindow::getOrderHeaderLabels() const
{
    return m_orderHeaderLabels;
}

GlobalConfigDialog *MainWindow::getGlobalConfigDialog() const
{
    return (GlobalConfigDialog*)&m_globalConfigDialog;
}



QStringList MainWindow::getManagedAccounts() const
{
    return m_managedAccounts;
}


