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
#include "ui_logdialog.h"
#include "contractdetailswidget.h"
#include "globalconfigdialog.h"
#include "orderstablewidget.h"
#include "security.h"
#include "portfoliotablewidget.h"
#include "helpers.h"

#include "ui_welcomedialog.h"
#include "welcomedialog.h"
#include "tablewidgetitem.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtDebug>
#include <QDateTime>
#include <QPlainTextEdit>

#include <iostream>




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ibClient(NULL)
    , m_numConnectionAttempts(0)
{
    QTimer::singleShot(0, this, SLOT(onWelcome()));

    connect(&m_saveSettingsTimer, SIGNAL(timeout()), this, SLOT(onSaveSettings()));
    m_saveSettingsTimer.start(1000*60*5);

    ui->setupUi(this);

    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->setVisible(false);
    ui->tabWidget->tabBar()->tabButton(1, QTabBar::RightSide)->setVisible(false);
    ui->tabWidget->tabBar()->tabButton(2, QTabBar::RightSide)->setVisible(false);

    ui->tabWidget->setTabsClosable(true);

    for (int i =0;i<ui->tabWidget->count();++i) {
        m_pairTabPageMap.insert(i, NULL);
    }

    pDebug(m_pairTabPageMap);

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(onTabCloseRequested(int)));

    OrdersTableWidget* tab = ui->ordersTableWidget;
    tab->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tab->horizontalHeader()->setStretchLastSection(true);

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
                      << "PercentChange2"
                      << "NetPercentChange"
                      << "NetProfit";


    tab->setEnabled(true);
    tab->setVisible(true);
    tab->setColumnCount(m_orderHeaderLabels.size());
    tab->setHorizontalHeaderLabels(m_orderHeaderLabels);
//    tab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab->horizontalHeader()->setSectionsMovable(true);
    tab->setShowGrid(true);

    connect(tab, SIGNAL(contextMenuEventTriggered(QPoint,QPoint)),
            this, SLOT(onOrdersTableContextMenuEventTriggered(QPoint,QPoint)));

    PortfolioTableWidget* ptw = ui->portfolioTableWidget;
    ptw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ptw->horizontalHeader()->setStretchLastSection(true);

    m_portfolioHeaderLabels
                     << "Symbol"
                     << "Position"
                     << "MarketPrice"
                     << "MarketValue"
                     << "AverageCost"
                     << "UnrealizedPNL"
                     << "RealizedPNL"
                     << "AccountName";

    ptw->setEnabled(true);
    ptw->setVisible(true);
    ptw->setColumnCount(m_portfolioHeaderLabels.size());
    ptw->setHorizontalHeaderLabels(m_portfolioHeaderLabels);
//    ptw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ptw->horizontalHeader()->setSectionsMovable(true);
    ptw->setShowGrid(true);

//    ui->portfolioTableWidget->setVisible(false);
//    ui->portfolioTab->setVisible(false);


    QTableWidget* homeTableWidget = ui->homeTableWidget;
    homeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    homeTableWidget->horizontalHeader()->setStretchLastSection(true);
    homeTableWidget->verticalHeader()->setVisible(false);


    m_headerLabels << "Pair"
            << "Price1"
            << "Price2"
            << "Ratio"
            << "RatioMA"
            << "RatioStdDev"
            << "PcntFromRatioMA"
            << "Correlation"
            << "RatioVolatility"
            << "RatioRSI"
            << "SpreadRSI";

    homeTableWidget->setColumnCount(m_headerLabels.size());
    homeTableWidget->setHorizontalHeaderLabels(m_headerLabels);

//    homeTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    homeTableWidget->setShowGrid(true);

    connect(ui->tabWidget->tabBar(), SIGNAL(tabMoved(int,int)),
            this, SLOT(onHomeTabMoved(int,int)));

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

//    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    QString sym1 = page->getUi()->pair1ContractDetailsWidget->getUi()->symbolLineEdit->text();
    QString sym2 = page->getUi()->pair2ContractDetailsWidget->getUi()->symbolLineEdit->text();
    QString tabSymbol = sym1 + "/" + sym2;
    for (int i=0;i<m_pairTabPageMap.count();++i) {
        PairTabPage* p = m_pairTabPageMap.values().at(i);
        if (p==NULL)
            continue;
        if (p->getTabSymbol() == tabSymbol)
            return;
    }

//    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    page->setPairTabPageId(ui->tabWidget->count()-1);
    page->setTabSymbol();
    ui->tabWidget->addTab(page, tabSymbol);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    m_pairTabPageMap.insert(ui->tabWidget->count()-1, page);

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
    connect(m_ibClient, SIGNAL(openOrderEnd()),
            this, SLOT(onOpenOrderEnd()));
    connect(m_ibClient, SIGNAL(updatePortfolio(Contract,int,double,double,double,double,double,QByteArray)),
            this, SLOT(onUpdatePortfolio(Contract,int,double,double,double,double,double,QByteArray)));
    connect(m_ibClient, SIGNAL(updateAccountTime(QByteArray)),
            this, SLOT(onUpdateAccountTime(QByteArray)));
    connect(m_ibClient, SIGNAL(tickPrice(long,TickType,double,int)),
            this, SLOT(onTickPrice(long,TickType,double,int)));
//    connect(m_ibClient, SIGNAL(tickSize(long,TickType,int)),
//            this, SLOT(onTickSize(long,TickType,int)));

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
//qDebug() << "[DEBUG-onManagedAccounts]" << m_managedAccounts;
    m_globalConfigDialog.setMangagedAccounts(m_managedAccounts);
    ui->actionGlobal_Config->setEnabled(true);

    m_ibClient->reqOpenOrders();

    m_ibClient->reqAccountUpdates(true, "DU210791");

    readPageSettings();
}

void MainWindow::onIbError(const int id, const int errorCode, const QByteArray errorString)
{
//    qDebug() << "IbError:" << id << errorCode << errorString.data();
    QPlainTextEdit* pte = m_logDialog.getUi()->logPlainTextEdit;
    QString errType("");
    if (errorCode != 2106)
        errType = "[IB_ERROR] ";
    else
        errType = "[IB_INFO_] ";
    QString msg(errType + QString::number(id) + QString(" ") + QString::number(errorCode) + QString(" ") + errorString);
    pte->appendPlainText(msg);

    statusBar()->clearMessage();
    statusBar()->showMessage(QString("        Log: ") + msg);

    if (errorCode == 200) {
        QMessageBox msgBox;
        msgBox.setText("IB Error");
        msgBox.setInformativeText(errorString);
//        enacted = true;
        /*int ret =*/ msgBox.exec();

    }
}

void MainWindow::onIbSocketError(const QString &error)
{
    static bool enacted = false;

    ++m_numConnectionAttempts;

    if (!enacted) {
        enacted = true;
        if (error == QString("Connection Refused")) {
            QMessageBox msgBox;
            msgBox.setText("Connection Refused");
            msgBox.setInformativeText("Please ensure TWS is running and accepts API calls.");
            enacted = true;
            /*int ret =*/ msgBox.exec();
        }
        else {
            if (m_numConnectionAttempts == 3) {
                QMessageBox msgBox;
                msgBox.setText(error + ". Please restart the application");
                msgBox.setInformativeText("Please ensure TWS is running and accepts API calls.");
                /*int ret =*/ msgBox.exec();
            }
            else {
                delay(1000);
                int clientId = 0;
                m_ibClient->connectToTWS("127.0.0.1", 7496, clientId);
                ++m_numConnectionAttempts;
            }
        }
        enacted = false;
    }
}

void MainWindow::onNextValidId(long orderId)
{
//qDebug() << "[DEBUG-onNextValidId] orderId:" << orderId;
    m_ibClient->setOrderId(orderId);
}

void MainWindow::onCurrentTime(long time)
{
    QDateTime dt(QDateTime::fromTime_t(time));

//qDebug() << "[DEBUG-OnCurrentTime]" << dt;
}

void MainWindow::onTwsConnected()
{
    ui->actionConnect_To_TWS->setEnabled(false);
    ui->actionConnect_To_TWS->setText("TWS Connected");
    ui->action_New->setEnabled(true);

//    QSettings settings;

//    int numTabPages = settings.value("numPairTabPages", 0).toInt();

//    for (int i=0;i<numTabPages;++i) {
//        PairTabPage* p = new PairTabPage(m_ibClient, m_managedAccounts, this);
//        p->readSettings();

//        QString tabSymbol = p->getTabSymbol();

//        ui->tabWidget->addTab(p, tabSymbol);
//        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
//        m_pairTabPageMap[ui->tabWidget->currentIndex()] = p;
//    }
//    delay(3000);
    m_ibClient->reqCurrentTime();
    m_numConnectionAttempts = 0;
}

void MainWindow::onTwsConnectionClosed()
{
    ui->actionConnect_To_TWS->setEnabled(true);
}

void MainWindow::onTabCloseRequested(int idx)
{

    pDebug(idx);

    QMap<int, PairTabPage*> tmpMap;


    // hack to fix missing map item


    PairTabPage* p = m_pairTabPageMap.value(idx);

    pDebug(m_pairTabPageMap);

    if (!p) {
        pDebug("");
        for (int i=3;i<m_pairTabPageMap.count();++i) {
            QString mapString("key: "
                              + QString::number(m_pairTabPageMap.keys().at(i))
                              + "value: "
                              + m_pairTabPageMap.values().at(i)->getTabSymbol());
            pDebug(QString("NO m_pairTabPageMap: " + mapString));
        }
        return;
    }


    if (p->reqClosePair()) {
        pDebug(2);

        // remove tabWidget tab
        QWidget* w = ui->tabWidget->widget(idx);
        ui->tabWidget->removeTab(idx);
        for (int i=0;i<m_pairTabPageMap.count()-1;++i) {
            if (i<idx) {
                tmpMap.insert(i, m_pairTabPageMap.value(i));
                continue;
            }
            else {
                tmpMap.insert(i, m_pairTabPageMap.value(i+1));
            }
        }
        delete w;
        m_pairTabPageMap = tmpMap;
        pDebug(m_pairTabPageMap);
    }


    pDebug("leaving");

//    // else ... POP UP WINDOW THAT THERE ARE ORDERS ACTIVE
}

void MainWindow::onOrderStatus(long orderId, const QByteArray &status, int filled, int remaining, double avgFillPrice,
                                int permId, int parentId, double lastFillPrice, int clientId, const QByteArray &whyHeld)
{
//qDebug() << "[DEBUG-onOrderStatus]"
//             << orderId
//             << status
//             << filled
//             << remaining
//             << avgFillPrice
//             << permId
//             << parentId
//             << lastFillPrice
//             << clientId
//             << whyHeld;

    // THIS IS TEMPORARY  ... FIX ME !!!!
    if (status == "PendingCancel" || status == "Cancelled")
        return;

    if (!ui->ordersTab->isEnabled())
        ui->ordersTab->setEnabled(true);
    if (!ui->ordersTab->isVisible())
        ui->ordersTab->setVisible(true);
    if (!ui->ordersTableWidget->isEnabled())
        ui->ordersTableWidget->setEnabled(true);
    if (!ui->ordersTableWidget->isVisible())
        ui->ordersTableWidget->setVisible(true);

    bool isS1 = false;
    bool isS2 = false;

    PairTabPage* p = NULL;
    Security* s1 = NULL;
    Security* s2 = NULL;

    bool isNewRow = true;

    int row = -1;

    QTableWidgetItem* item = NULL;
    QTableWidgetItem* sym1Item = NULL;
    QTableWidgetItem* sym2Item = NULL;

    // get the page and securities

    int cnt = m_pairTabPageMap.count();
    for (int i=0;i<cnt;++i) {
        if (m_pairTabPageMap.value(i) == NULL)
            continue;
        p = m_pairTabPageMap.values().at(i);

//        QString tabSymbol = p->getTabSymbol();

        s1 = p->getSecurities().at(0);
        s2 = p->getSecurities().at(1);

        if (s1->getSecurityOrderMap()->contains(orderId)) {
            isS1 = true;
            break;
        }
        else if (s2->getSecurityOrderMap()->contains(orderId)) {
            isS2 = true;
            break;
        }
    }

    // is this an order initiated from TWS interface?
    if (!isS1 && !isS2) {
        // FIXME.. I need to address this !!!!!!!!!!!!!!!!
//qDebug() << "[DEBUG-onOrderStatus] WARNING: orderId (" << orderId << ") not known.. is it from TWS?";
        return;
    }

    // is this a new row?
    for (int r=0;r<ui->ordersTableWidget->rowCount();++r) {

        for (int c=0;c<ui->ordersTableWidget->columnCount();++c) {
            if (ui->ordersTableWidget->horizontalHeaderItem(c)->text() == "Pair")
                item = ui->ordersTableWidget->item(r,c);
        }

        if (p->getTabSymbol() == item->text()) {
            isNewRow = false;
            row = r;
            break;
        }
    }

    // create a new row if needed
    if (isNewRow) {
        ui->ordersTableWidget->setRowCount(ui->ordersTableWidget->rowCount()+1);
        row = ui->ordersTableWidget->rowCount() - 1;
        for (int c=0;c<ui->ordersTableWidget->columnCount();++c) {
            ui->ordersTableWidget->setItem(row, c, new TableWidgetItem);
        }
    }

    // set orderTableWidget data
    for (int c=0;c<ui->ordersTableWidget->columnCount();++c) {
//        bool isShort = false;
        item = ui->ordersTableWidget->item(row, c);
        QString headerString = ui->ordersTableWidget->horizontalHeaderItem(c)->text();

//        m_orderHeaderLabels << "Pair"
//                          << "Sym1"
//                          << "Sym2"
//                          << "Size1"
//                          << "Size2"
//                          << "Cost/Unit1"
//                          << "Cost/Unit2"
//                          << "TotalCost1"
//                          << "TotalCost2"
//                          << "Last1"
//                          << "Last2"
//                          << "Diff1"
//                          << "Diff2"
//                          << "PercentChange1"
//                          << "PercentChange2";

        if (headerString == "Pair") {
            QString tabSymbol = p->getTabSymbol();
            item->setText(tabSymbol);
        }
        else if (headerString == "Sym1") {
            if (isS1) {
                QString sym1 = s1->contract()->symbol;
                item->setText(sym1);
                sym1Item = item;
            }
            else
                continue;
        }
        else if (headerString == "Sym2") {
            if (isS2) {
                QString sym2 = s2->contract()->symbol;
                item->setText(sym2);
                sym2Item = item;
            }
            else
                continue;
        }
        else if (headerString == "Size1") {
            if (isS1) {
                if (s1->getSecurityOrderMap()->count() == 2) {
                    int size1 = item->text().toInt();
                    if (size1 > 0)
                        item->setText(QString::number(size1-filled));
                    else
                        item->setText(QString::number(size1+filled));
                }
                else {
                    if (s1->getSecurityOrderMap()->values().at(0)->order.action == "SELL") {
                        item->setText(QString::number(-filled));
                        item->setTextColor(QColor(Qt::red));
                        if (sym1Item)
                            sym1Item->setTextColor(QColor(Qt::red));
//                        isShort = true;
                    }
                    else
                        item->setText(QString::number(filled));
                }
            }
            else
                continue;
        }
        else if (headerString == "Size2") {
            if (isS2)
                if (s2->getSecurityOrderMap()->count() == 2) {
                    int size2 = item->text().toInt();
                    if (size2 > 0)
                        item->setText(QString::number(size2-filled));
                    else
                        item->setText(QString::number(size2+filled));
                }
                else {
                    if (s2->getSecurityOrderMap()->values().at(0)->order.action == "SELL") {
                        item->setText(QString::number(-filled));
                        item->setTextColor(QColor(Qt::red));
                        if (sym2Item)
                            sym2Item->setTextColor(QColor(Qt::red));
//                        isShort = true;
                    }
                    else
                        item->setText(QString::number(filled));
            }
            else
                continue;
        }
        else if (headerString == "Cost/Unit1") {
            if (isS1)
                item->setText(QString::number(avgFillPrice,'f',2));
            else
                continue;
        }
        else if (headerString == "Cost/Unit2") {
            if (isS2)
                item->setText(QString::number(avgFillPrice,'f',2));
            else
                continue;
        }
        else if (headerString == "TotalCost1") {
            if (isS1)
                item->setText(QString::number(filled * avgFillPrice,'f',2));
            else
                continue;
        }
        else if (headerString == "TotalCost2") {
            if (isS2)
                item->setText(QString::number(filled * avgFillPrice,'f',2));
            else
                continue;
        }
    }

    // update the security's orderstate
    Security* s;
    if (isS1) {
        s = s1;
    }
    else
        s = s2;
//    SecurityOrder* so = (*(s->getSecurityOrderMap())xzc)[orderId];
    SecurityOrder* so = s->getSecurityOrderMap()->value(orderId);
    so->status = status;
    so->filled = filled;
    so->remaining = remaining;
    so->avgFillPrice = avgFillPrice;
    so->permId = permId;
    so->parentId = parentId;
    so->lastFillPrice = lastFillPrice;
    so->clientId = clientId;
    so->whyHeld = whyHeld;

    // has the order been closed?

    if (so->triggerType == EXIT) {
        QMap<long, SecurityOrder *>* securityOrderMap = s->getSecurityOrderMap();
        if (!securityOrderMap)
            return;

        // This is a hack to fix the security order map not containing the referenceOrderId crash.
        if (!securityOrderMap->contains(so->referenceOrderId)) {
            pDebug("so->referenceOrderId NOT FOUND");
            return;
        }

        int sFilled = securityOrderMap->value(so->referenceOrderId)->filled;
        // I had a crash from the line below... use sFilled now to be able to trace the crash.
        //if (so->filled == (*(s->getSecurityOrderMap()))[so->referenceOrderId]->filled) {
        if (so->filled == sFilled) {
            ui->ordersTableWidget->removeRow(row);
            for (int i=ui->portfolioTableWidget->rowCount()-1;i>=0;--i) {
                ui->portfolioTableWidget->removeRow(i);
            }
//            (*(s->getSecurityOrderMap())).remove(so->referenceOrderId);
            s->getSecurityOrderMap()->remove(so->referenceOrderId);
//            (*(s->getSecurityOrderMap())).remove(so->order.orderId);
            s->getSecurityOrderMap()->remove(so->order.orderId);
            p->setExitingOrder(false);
        }
    }
}


void MainWindow::onOpenOrder(long orderId, const Contract &contract, const Order &order, const OrderState &orderState)
{
    Q_UNUSED(contract);
//qDebug() << "[DEBUG-onOpenOrder]"
//             << orderId
//             << contract.symbol
//             << order.action
//             << orderState.initMargin
//             << orderState.maintMargin
//             << orderState.status;

    for (int i=0;i<m_pairTabPageMap.count();++i) {
        if (m_pairTabPageMap.value(i) == NULL)
            continue;
        PairTabPage* p = m_pairTabPageMap.values().at(i);
        for (int j=0;j<p->getSecurities().count();++j) {
            Security* s = p->getSecurities().at(j);
            if (!s->getSecurityOrderMap()->contains(orderId))
                continue;
            SecurityOrder* so = (*(s->getSecurityOrderMap()))[orderId];
            so->order = order;
            so->orderState = orderState;
        }
//        QString tabSymbol = p->getTabSymbol();
    }
}

void MainWindow::onOpenOrderEnd()
{
//qDebug() << "[DEBUG-onOpenOrderEnd]";
}

void MainWindow::onUpdatePortfolio( const Contract& contract, int position,
                                    double marketPrice, double marketValue, double averageCost,
                                    double unrealizedPNL, double realizedPNL, const QByteArray& accountName)
{
    if (position == 0)
        return;

//qDebug() << "[DEBUG-onUpdatePortfolio]"
//             << contract.symbol << " (" << contract.conId << ") "
//             << position
//             << marketPrice
//             << marketValue
//             << averageCost
//             << unrealizedPNL
//             << realizedPNL
//             << accountName;


//    // update orders table
//    QString sym(contract.symbol);
//    double last = marketPrice;

//    for (int i=0;i<m_pairTabPageMap.values().count();++i) {
//        PairTabPage* p = m_pairTabPageMap.values().at(i);
//        if (p==NULL)
//            continue;
//        for (int j=0;j<p->getSecurities().count();++j) {
//            Security* s = p->getSecurities().at(j);
//            if (s->contract()->symbol == sym) {
//                updateOrdersTable(sym, last);
//            }
//        }
//    }


    // update portfolio table
    int row = -1;
    bool isUpdate = false;

    int rowCount = ui->portfolioTableWidget->rowCount();
    int colCount = ui->portfolioTableWidget->columnCount();

    if (rowCount) {
        for (int r=0;r<rowCount;++r) {
            for (int c=0;c<colCount;++c) {
                if (ui->portfolioTableWidget->horizontalHeaderItem(c)->text() == "Symbol") {
                    if (ui->portfolioTableWidget->item(r,c)->text() == contract.symbol) {
                        row = r;
                        isUpdate = true;
                    }
                }
            }
        }
    }

//    << "Symbol"
//    << "Position"
//    << "MarketPrice"
//    << "MarketValue"
//    << "AverageCost"
//    << "UnrealizedPNL"
//    << "RealizedPNL"
//    << "AccountName";

    TableWidgetItem* i = NULL;

    if (!isUpdate) {
        row = ui->portfolioTableWidget->rowCount();
        ui->portfolioTableWidget->setRowCount(ui->portfolioTableWidget->rowCount()+1);
    }


    for (int c=0;c<ui->portfolioTableWidget->horizontalHeader()->count();++c) {

        QString field = ui->portfolioTableWidget->horizontalHeaderItem(c)->text();

        if (!isUpdate) {
            i = new TableWidgetItem;
            ui->portfolioTableWidget->setItem(row, c, i);
        }
        else {
            i = (TableWidgetItem*)ui->portfolioTableWidget->item(row, c);
        }
        if (field == "Symbol")
            i->setText(contract.symbol);
        else if (field == "Position")
            i->setText(QString::number(position));
        else if (field == "MarketPrice")
            i->setText(QString::number(marketPrice, 'f', 2));
        else if (field == "MarketValue")
            i->setText(QString::number(marketValue, 'f', 2));
        else if (field == "AverageCost")
            i->setText(QString::number(averageCost, 'f', 2));
        else if (field == "UnrealizedPNL")
            i->setText(QString::number(unrealizedPNL, 'f', 2));
        else if (field == "RealizedPNL")
            i->setText(QString::number(realizedPNL, 'f', 2));
        else if (field == "AccountName")
            i->setText(accountName);
        else
            continue;
    }
//qDebug() << "[DEBUG-onUpdatePortfolio] END";
}

void MainWindow::onUpdateAccountTime(const QByteArray &timeStamp)
{
    Q_UNUSED(timeStamp);
//    qDebug() << "[DEBUG-onUpdateAccountTime]" << QDateTime::fromTime_t(timeStamp.toUInt()).toString("yyMMdd::hh:mm:ss");
}

void MainWindow::onAccountDownloadEnd(const QByteArray &accountName)
{
    Q_UNUSED(accountName);
//qDebug() << "[DEBUG-onAccountDownloadEnd]" << accountName;
}


void MainWindow::writeSettings()
{
//    qDebug() << "[DEBUG-MainWindow::writeSettings]";

    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("state", saveState());
    settings.setValue("numPairTabPages", ui->tabWidget->count()-3);
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
        else if (ui->tabWidget->tabText(i) == QString("Portfolio")) {
            ++subtract;
            continue;
        }
        settings.setArrayIndex(i-subtract);
        PairTabPage* page = qobject_cast<PairTabPage*>(ui->tabWidget->widget(i));
        if (!page)
            continue;
        QString tabSymbol = page->getTabSymbol();
//qDebug() << "[DEBUG-MainWindow::writeSettings] tabSymbol" << i << ":" << tabSymbol;
        settings.setValue("tabSymbol", tabSymbol);
        page->writeSettings();
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
//    qDebug() << "[DEBUG-" << __func__ << "] leaving";
}

void MainWindow::readSettings()
{
//    P_DEBUG;

    QSettings settings;
    settings.beginGroup("mainwindow");

    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();

    QSize size = settings.value("size", QSize(400, 600)).toSize();

    QByteArray state = settings.value("state", QByteArray())
                                                  .toByteArray();

    settings.endGroup();

   restoreState(state);
   resize(size);
   move(pos);



}

void MainWindow::readPageSettings()
{
    pDebug("");

    QSettings s;
    s.beginGroup("mainwindow");
    int size = s.beginReadArray("pages");

//qDebug() << "[DEBUG-readPageSettings] size" << size;

    for (int i=0;i<size;++i) {

        s.setArrayIndex(i);
        PairTabPage* p = new PairTabPage(m_ibClient, m_managedAccounts, this);
        m_pairTabPageMap.insert(ui->tabWidget->count(), p);
        QString tabSymbol = s.value("tabSymbol").toString();
//qDebug() << "[DEBUG-readPageSettings] tabSymbol" << i << ":" << tabSymbol;
        p->setTabSymbol(tabSymbol);
        p->readSettings();

        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

        ui->tabWidget->addTab(p, p->getTabSymbol());
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
//        m_pairTabPageMap[ui->tabWidget->currentIndex()] = p;
    }
    s.endArray();
    ui->tabWidget->setCurrentIndex(s.value("tabWidgetCurrentIndex").toInt());
    s.endGroup();

    pDebug("leaving");
}


QStringList MainWindow::getHeaderLabels() const
{
    return m_headerLabels;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        widget->close();
    }
    event->accept();
}


void MainWindow::on_actionGlobal_Config_triggered()
{
    m_globalConfigDialog.exec();
}

void MainWindow::on_action_Log_Dialog_triggered()
{
    m_logDialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    m_logDialog.show();
}

void MainWindow::onOrdersTableContextMenuEventTriggered(const QPoint &pos, const QPoint &globalPos)
{
    m_ordersTableRowPoint = pos;

    QMenu m;
    m.addAction("Close Order", this, SLOT(onCloseOrder()));
    m.exec(globalPos);
}

void MainWindow::onCloseOrder()
{
    int row = ui->ordersTableWidget->itemAt(m_ordersTableRowPoint)->row();

    PairTabPage* p;
    QString tabSymbol;

    for (int c=0;c<ui->ordersTableWidget->columnCount();++c) {
        QTableWidgetItem* hi = ui->ordersTableWidget->horizontalHeaderItem(c);
        if ( hi->text() == QString("Pair")) {
            tabSymbol = ui->ordersTableWidget->item(row, c)->text();
            break;
        }
    }

    for (int i=0;i<m_pairTabPageMap.values().size();++i) {
        if (m_pairTabPageMap.value(i) == NULL)
            continue;
        p = m_pairTabPageMap.values().at(i);
        if (p->getTabSymbol() == tabSymbol) {
            p->exitOrder();
        }
    }
}

void MainWindow::onHomeTabMoved(int from, int to)
{
//qDebug() << "[DEBUG-onHomeTabMoved] from:" << from << "to:" << to;

    QMap<int, PairTabPage*> tmpMap;
    for (int i=0;i<m_pairTabPageMap.count();++i) {
        if (from > to) {
            if (i < to && i > from) {
                tmpMap.insert(i, m_pairTabPageMap.value(i));
                continue;
            }
            else {
                if (i == to) {
                    tmpMap.insert(i, m_pairTabPageMap.value(from));
                    continue;
                }
                if (i > to) {
                    tmpMap.insert(i, m_pairTabPageMap.value(i+1));
                    continue;
                }
            }
        }
        else {
            if (i < from && i > to) {
                tmpMap.insert(i, m_pairTabPageMap.value(i));
                continue;
            }
            else {
                if (i < to) {
                    tmpMap.insert(i, m_pairTabPageMap.value(i+1));
                    continue;
                }
                if (i == to) {
                    tmpMap.insert(i, m_pairTabPageMap.value(from));
                    continue;
                }
            }
        }
    }
    m_pairTabPageMap = tmpMap;
}

void MainWindow::onSaveSettings()
{
    writeSettings();
}

void MainWindow::onTickPrice(const long &tickerId, const TickType &field, const double &price, const int &canAutoExecute)
{
//    qDebug() << "";
//    qDebug() << "";

    Q_UNUSED(canAutoExecute);

    QList<Security*> securities;

    int cnt = 0;

    switch (field)
    {
    case LAST:
        // pDebug("1");
        cnt = PairTabPage::RawDataMap.values().count();
        for (int i=0;i<cnt;++i) {
            Security* value = PairTabPage::RawDataMap.values().at(i);
            if (tickerId == PairTabPage::RawDataMap.key(value)) {
                securities.append(value);
            }
        }
        // pDebug(securities);

        // updateOrdersPage
        if (!securities.isEmpty()) {
            updateOrdersTable(securities.first()->contract()->symbol, price);
        }

        for (int i=0;i<securities.count();++i) {
            // pDebug(2);
            Security* ss = securities.at(i);
            for (int j=0;j<m_pairTabPageMap.values().count();++j) {
                // pDebug(3);
                bool containsSecurity = false;
                bool canCheckTradeExits = false;
                Security* s = NULL;
                PairTabPage* p = m_pairTabPageMap.values().at(j);
                if (!p)
                    continue;
//                if (p->getSecurities().contains(s)
//                        && p->isTrading(s))
                for (int k=0;k<p->getSecurities().count();++k) {
                    // pDebug(4);
                    s = p->getSecurities().at(k);
                    if (s->contract()->symbol == ss->contract()->symbol) {
                        if (s->contract()->secType == QByteArray("FUT")) {
                            if (s->contract()->expiry == ss->contract()->expiry)
                            {
                                containsSecurity = true;
                                break;
                            }
                        }
                        else {
                            containsSecurity = true;
                            break;
                        }
//                        if (containsSecurity && !s->getSecurityOrderMap()->isEmpty()) {
//                            for (int i=0;i<s->getSecurityOrderMap()->count();++i) {
//                                SecurityOrder* so = s->getSecurityOrderMap()->values().at(i);
//                                if (so->triggerType != EXIT) {
//                                    canCheckTradeExits = true;
//                                }
//                            }
//                        }
                    }
                }
                // pDebug(5);
                if (containsSecurity && p->isTrading(s)) {
                    // pDebug(s);
                    // pDebug(price);
                    s->appendRawPrice(price);
                    p->appendPlotsAndTable(p->getSecurityMap().key(s));

                    if (!p->getUi()->manualTradeEntryCheckBox->isChecked()
                            && !p->getUi()->activateButton->isEnabled()
                            && p->getUi()->deactivateButton->isEnabled())
                    {
                        p->checkTradeTriggers();
                    }
                    if (!p->getUi()->manualTradeExitCheckBox->isChecked()
                            && !p->getUi()->activateButton->isEnabled()
                            && p->getUi()->deactivateButton->isEnabled())
                    {
                        for (int i=0;i<s->getSecurityOrderMap()->count();++i) {
                            SecurityOrder* so = s->getSecurityOrderMap()->values().at(i);
                            if (so->triggerType != EXIT) {
                                canCheckTradeExits = true;
                                break;
                            }
                        }
                        if (canCheckTradeExits) {
                            p->checkTradeExits(price);
                        }
                    }
                }
                // pDebug(6);
            }
        }
        break;
    default:
        break;
    }

    if (ui->portfolioTableWidget->rowCount() == 1) {
        ui->portfolioTableWidget->removeRow(0);
    }
}

void MainWindow::onTickSize(const long &tickerId, const TickType &field, const int &size)
{
    Security* s = NULL;
    bool breakout = false;

    for (int i=0;i<m_pairTabPageMap.count();++i) {
        PairTabPage* p = m_pairTabPageMap.values().at(i);
        if (p==NULL)
            continue;
        QList<Security*> securities = p->getSecurities();
        for (int j=0;j<securities.size();++j) {
            Security* ss = securities.at(j);
            if (tickerId == ss->getRealTimeTickerId()) {
                s = ss;
                breakout = true;
                break;
            }
        }
        if (breakout)
            break;
    }

    if (!s) {
//qDebug() << "[ERROR]" << __PRETTY_FUNCTION__ << __LINE__ << "where is the tickerId???";
    }

    switch (field)
    {
    case LAST_SIZE:
//qDebug() << "[DEBUG]" << __func__ << __LINE__ << "SIZE:" << size;
        s->appendRawSize(size);
        break;
    default:
        break;
    }
}

void MainWindow::onWelcome()
{
    connect(&m_welcomeTimer, SIGNAL(timeout()), this, SLOT(onWelcomeTimeout()));
    m_welcomeTimer.start(1000);
    m_welcomeDialog = new WelcomeDialog;
    connect(m_welcomeDialog->getUi()->clearSettingsButton, SIGNAL(pressed()), this, SLOT(onClearSettings()));
    connect(m_welcomeDialog->getUi()->clickShowButtonsManually, SIGNAL(pressed()), this, SLOT(onClickShowButtonsManually()));
    connect(m_welcomeDialog->getUi()->startButton, SIGNAL(pressed()), this, SLOT(onWelcomeDialogStartButtonClicked()));
    m_welcomeDialog->exec();
    m_welcomeTimer.stop();
//    disconnect(m_welcomeDialog->getUi()->clearSettingsButton, SIGNAL(pressed()), this, SLOT(onClearSettings()));
}

void MainWindow::onWelcomeTimeout()
{
    int val = m_globalConfigDialog.getUi()->welcomeCountDownSpinBox->value();
    static int t = val;

    if (t==1) {
        m_welcomeDialog->close();
        delete m_welcomeDialog;
        return;
    }
    m_welcomeDialog->getUi()->countDownLabel->setText(QString::number(--t));
}

void MainWindow::onClearSettings()
{
    QSettings s;
    s.clear();
}

void MainWindow::onClickShowButtonsManually()
{
    PairTabPage::DontClickShowButtons = true;
}

void MainWindow::onWelcomeDialogStartButtonClicked()
{
    m_welcomeDialog->close();
    m_welcomeTimer.stop();
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


struct Info
{
    int row;
    bool isSym1;
    bool isSym2;
    int size1;
    int size2;
    bool sym1IsShort;
    bool sym2IsShort;
    double purchasePrice1;
    double purchasePrice2;
    double totalCost1;
    double totalCost2;
    double last1;
    double last2;
    double diff1;
    double diff2;
    double pcntChange1;
    double pcntChange2;
    double netProfit;
    double netPcnt;

    Info()
        : row(-1)
        , isSym1(false)
        , isSym2(false)
        , size1(0)
        , size2(0)
        , sym1IsShort(false)
        , sym2IsShort(false)
        , purchasePrice1(0)
        , purchasePrice2(0)
        , totalCost1(0)
        , totalCost2(0)
        , last1(0)
        , last2(0)
        , diff1(0)
        , diff2(0)
        , pcntChange1(0)
        , pcntChange2(0)
        , netProfit(0)
        , netPcnt(0)
    {}
};


void MainWindow::updateOrdersTable(const QString & symbol, const double & last)
{
// pDebug("");

    OrdersTableWidget* tw = ui->ordersTableWidget;

//    m_orderHeaderLabels << "Pair"
//                      << "Sym1"
//                      << "Sym2"
//                      << "Size1"
//                      << "Size2"
//                      << "Cost/Unit1"
//                      << "Cost/Unit2"
//                      << "TotalCost1"
//                      << "TotalCost2"
//                      << "Last1"
//                      << "Last2"
//                      << "Diff1"
//                      << "Diff2"
//                      << "PercentChange1"
//                      << "PercentChange2"
//                      << "NetDiff"
//                      << "NetPercentChange";
    QList<Info*> infoList;

    for (int r=0;r<tw->rowCount();++r) {
        Info* info = new Info;
        for (int c=0;c<tw->columnCount();++c) {
            QString field = tw->horizontalHeaderItem(c)->text();
            QString text = tw->item(r,c)->text();
            if (field == "Sym1" && text == symbol) {
                info->isSym1 = true;
            }
            else if (field == "Sym2" && text == symbol) {
                info->isSym2 = true;
            }
            else if (field == "Size1") {
                info->size1 = text.toInt();
                if (info->size1 < 0) {
                    info->sym1IsShort = true;
                }
            }
            else if (field == "Size2") {
                info->size2 = text.toInt();
                if (info->size2 < 0) {
                    info->sym2IsShort = true;
                }
            }
            else if (field == "Cost/Unit1") {
                info->purchasePrice1 = text.toDouble();
            }
            else if (field == "Cost/Unit2") {
                info->purchasePrice2 = text.toDouble();
            }
            else if (field == "TotalCost1") {
                info->totalCost1 = text.toDouble();
            }
            else if (field == "TotalCost2") {
                info->totalCost2 = text.toDouble();
            }
            else if (field == "Last1") {
                info->last1 = text.toDouble();
            }
            else if (field == "Last2") {
                info->last2 = text.toDouble();
            }
            else if (field == "Diff1") {
                info->diff1 = text.toDouble();
            }
            else if (field == "Diff2") {
                info->diff2 = text.toDouble();
            }
            else if (field == "PercentChange1") {
                info->pcntChange1 = text.toDouble();
            }
            else if (field == "PercentChange2") {
                info->pcntChange2 = text.toDouble();
            }
            else if (field == "NetProfit") {
                info->netProfit = text.toDouble();
            }
            else if (field == "NetPercentChange") {
                info->netPcnt = text.toDouble();
            }
        }
        infoList.append(info);
    }

    for (int r=0;r<infoList.count();++r) {
        Info* info = infoList.at(r);
        if (!(info->isSym1 || info->isSym2))
            continue;
        if (info->isSym1) {
            info->last1 = last;
            info->diff1 = info->purchasePrice1 - last;
            info->pcntChange1 = info->diff1 / info->purchasePrice1 * 100;
        }
        else {
            info->last2 = last;
            info->diff2 = info->purchasePrice2 - last;
            info->pcntChange2 = info->diff2 / info->purchasePrice2 * 100;
        }
    }

    for (int r=0;r<tw->rowCount();++r) {
        Info* info = infoList.at(r);
        if (!(info->isSym1 || info->isSym2)) {
            continue;
        }
        for (int c=0;c<tw->columnCount();++c) {
            QString field = tw->horizontalHeaderItem(c)->text();
            QTableWidgetItem* item = tw->item(r,c);
            if (info->isSym1) {
                if (field == "Last1") {
                    item->setText(QString::number(info->last1,'f',2));
                }
                else if (field == "Diff1") {
                    if (!info->sym1IsShort && info->diff1 > 0)
                    {
                        info->diff1 = info->diff1 * -1;
                        info->pcntChange1 = info->pcntChange1 * -1;
                    }
                    item->setText(QString::number(info->diff1,'f',2));
                    if (info->diff1 < 0)
                        item->setTextColor(QColor(Qt::red));
                    else
                        item->setTextColor(QColor(Qt::black));
                }
                else if (field == "PercentChange1") {
                    item->setText(QString::number(info->pcntChange1,'f',2));
                    if (info->pcntChange1 < 0)
                        item->setTextColor(QColor(Qt::red));
                    else
                        item->setTextColor(QColor(Qt::black));
                }
            }
            else {
                if (field == "Last2") {
                    item->setText(QString::number(info->last2,'f',2));
                }
                else if (field == "Diff2") {
                    if  (!info->sym2IsShort && info->diff2 > 0)
                    {
                        info->diff2 = info->diff2 * -1;
                        info->pcntChange2 = info->pcntChange2 * -1;
                    }
                    item->setText(QString::number(info->diff2,'f',2));
                    if (info->diff2 < 0)
                        item->setTextColor(QColor(Qt::red));
                    else
                        item->setTextColor(QColor(Qt::black));
                }
                else if (field == "PercentChange2") {
                    item->setText(QString::number(info->pcntChange2,'f',2));
                    if (info->pcntChange2 < 0)
                        item->setTextColor(QColor(Qt::red));
                    else
                        item->setTextColor(QColor(Qt::black));
                }
            }
            if (field == "NetProfit") {
//                double netDiff = info->diff1 + info->diff2;
                double netPcnt = info->pcntChange1 + info->pcntChange2;
                double netProfit = (info->totalCost1 + info->totalCost2) * netPcnt / 100;
                item->setText(QString::number(netProfit,'f',2));
                if (netProfit < 0)
                    item->setTextColor(QColor(Qt::red));
                else
                    item->setTextColor(QColor(Qt::black));
            }
            else if (field == "NetPercentChange") {
                double netPcnt = info->pcntChange1 + info->pcntChange2;
                item->setText(QString::number(netPcnt,'f',2));
                if (netPcnt < 0)
                    item->setTextColor(QColor(Qt::red));
                else
                    item->setTextColor(QColor(Qt::black));
            }
            else {
                continue;
            }
        }

    }

    qDeleteAll(infoList);
//qDebug() << "[DEBUG-updateOrdersTable] END";
}






