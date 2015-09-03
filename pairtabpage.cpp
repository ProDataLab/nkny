#include "pairtabpage.h"
#include "ui_pairtabpage.h"
#include "ui_mainwindow.h"
#include "ui_stddevlayertab.h"
#include "ui_contractdetailswidget.h"
#include "contractdetailswidget.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "qcustomplot.h"
#include "helpers.h"
#include "security.h"
#include "mainwindow.h"
#include "stddevlayertab.h"

#include <QDateTime>
#include <QTime>
#include <QSpinBox>
#include <QPair>
#include <QPen>
#include <QColor>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QLineEdit>


PairTabPage::PairTabPage(IBClient *ibClient, QWidget *parent)
    : QWidget(parent)
    , m_ibClient(ibClient)
    , ui(new Ui::PairTabPage)
{

//    qDebug() << "[DEBUG-PairTabPage]";

    ui->setupUi(this);

    mwui = qobject_cast<MainWindow*>(parent)->getUi();

    m_origButtonStyleSheet = ui->activateButton->styleSheet();

    QStringList secTypes;
    secTypes += "STK";
    secTypes += "FUT";
    secTypes += "OPT";
//    secTypes += "IND";
//    secTypes += "FOP";
//    secTypes += "CASH";
//    secTypes += "BAG";
//    secTypes += "NEWS";

//    qDebug() << "[DEBUG-PairTabPage]" << secTypes;


    m_pair1ContractDetailsWidget = ui->pair1ContractDetailsWidget;
    m_pair2ContractDetailsWidget = ui->pair2ContractDetailsWidget;

    Ui::ContractDetailsWidget* cdui1 = m_pair1ContractDetailsWidget->getUi();
    Ui::ContractDetailsWidget* cdui2 = m_pair2ContractDetailsWidget->getUi();

    cdui1->securityTypeComboBox->addItems(secTypes);
    cdui2->securityTypeComboBox->addItems(secTypes);


    cdui1->symbolLineEdit->setText("A");
    cdui1->primaryExchangeLineEdit->setText("NYSE");

    cdui2->symbolLineEdit->setText("AFFX");
    cdui2->primaryExchangeLineEdit->setText("NASDAQ");

    ui->pairsTabWidget->tabBar()->setTabText(0, cdui1->symbolLineEdit->text());
    ui->pairsTabWidget->tabBar()->setTabText(1, cdui2->symbolLineEdit->text());

    connect(cdui1->symbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui1SymbolTextChanged(QString)));
    connect(cdui2->symbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui2SymbolTextChanged(QString)));

    connect(ui->activateButton, SIGNAL(clicked(bool)),
            this, SLOT(onActivateButtonClicked(bool)));
    connect(ui->deactivateButton, SIGNAL(clicked(bool)),
            this, SLOT(onDeactivateButtonClicked(bool)));
    connect(m_ibClient, SIGNAL(orderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)),
            this, SLOT(onOrderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)));
    connect(m_ibClient, SIGNAL(openOrder(long,Contract,Order,OrderState)),
            this, SLOT(onOpenOrder(long,Contract,Order,OrderState)));
    connect(m_ibClient, SIGNAL(contractDetails(int,ContractDetails)),
            this, SLOT(onContractDetails(int,ContractDetails)));
    connect(m_ibClient, SIGNAL(historicalData(long,QByteArray,double,double,double,double,int,int,double,int)),
            this, SLOT(onHistoricalData(long,QByteArray,double,double,double,double,int,int,double,int)));
    connect(ui->tradeEntryNumStdDevLayersSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onTradeEntryNumStdDevLayersChanged(int)));
    connect(ui->waitCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(onWaitCheckBoxStateChanged(int)));

}

PairTabPage::~PairTabPage()
{
//    if (!m_securityMap.keys().isEmpty()) {
//        foreach(Security* s, m_securityMap.values()) {
//            delete s;
//        }
//    }
}

void PairTabPage::onHistoricalData(long reqId, const QByteArray& date, double open, double high,
    double low, double close, int volume, int barCount, double WAP, int hasGaps)
{
    if (!(m_securityMap.keys().contains(reqId) || m_newBarMap.keys().contains(reqId)))
        return;

    Security* s= NULL;
    long sid = 0;
    double timeStamp = 0;
    bool isNewBarReq = false;

    if (m_timeFrame == DAY_1)
        timeStamp = (double)QDateTime::fromString(date, "yyyyMMdd").toTime_t();
    else
        timeStamp = date.toDouble();


    if (m_securityMap.keys().contains(reqId)) {
        s = m_securityMap[reqId];
        sid = reqId;
    }
    else if (m_newBarMap.values().contains(reqId)) {
        isNewBarReq = true;
        sid = m_newBarMap.key(reqId);
        s = m_securityMap[sid];
    }
    else {
        qDebug() << "[ERROR-onHistoricalData] request Id UNKNOWN!";
        qDebug() << "    secMapKeys:" << m_securityMap.keys();
        qDebug() << "    secMapVals:" << m_securityMap.values();
        qDebug() << "    reqId:" << reqId;
    }

    if (!isNewBarReq) {
        if (date.startsWith("finished")) {
            double lastBarsTimeStamp = s->getHistData(m_timeFrame)->timeStamp.last();
            s->setLastBarsTimeStamp(lastBarsTimeStamp);

            s->getTimer()->start(m_timeFrameInSeconds * 1000);

            DataVecsHist* dvh = s->getHistData(m_timeFrame);
            if (dvh->timeStamp.size() > 250) {
                s->fixHistDataSize(m_timeFrame);
            }

            showPlot(sid, LINE);

            if (m_securityMap.keys().indexOf(reqId) == 1) {
                plotRatio();
                plotRatioMA();
                plotRatioStdDev();
                plotRatioPercentFromMean();
                plotCorrelation();
//                plotCointegration();
                plotRatioVolatility();
                plotRatioRSI();
                addTableRow();
            }
        }
        else {
//            qDebug() << "[DEBUG-onHistData] reqId:" << reqId;
//            qDebug() << "                   tickerId:" << m_securityMap.key(s);
//            qDebug() << "                   contract-symbol:" << s->contract()->symbol;
//            qDebug();
            s->appendHistData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
        }
    }
    else if (isNewBarReq){
        if (date.startsWith("finished")) {
            s->handleNewBarData(m_timeFrame);
            if (ui->activateButton->isEnabled()) {
                checkTradeTriggers();
                if (!m_orders.isEmpty())
                    checkTradeExits();
            }
            appendPlot(sid);
        }
        else {
            s->appendNewBarData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
        }
    }
}

//void PairTabPage::on_pair1SymbolLineEdit_textEdited(const QString &arg1)
//{
//    m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->setText(arg1.toUpper());
//    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
//                                m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text()
//                                + "/"
//                                + m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text());
//}

//void PairTabPage::on_pair2SymbolLineEdit_textEdited(const QString &arg1)
//{
//    m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->setText(arg1.toUpper());
//    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
//                                m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text()
//                                + "/"
//                                + m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text());
//}

//void PairTabPage::on_pair1SymbolLineEdit_editingFinished()
//{
//    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
//                                m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text() + "/" + m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text());
//    ui->pairsTabWidget->setTabText(0, m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text());
//}

//void PairTabPage::on_pair2SymbolLineEdit_editingFinished()
//{
//    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
//                                m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text() + "/" + m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text());
//    ui->pairsTabWidget->setTabText(1, m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text());
//}

void PairTabPage::on_pair1ShowButton_clicked()
{
    if (m_securityMap.keys().size() >= 2) {
        qDebug() << "[DEBUG-on_pair1ShowButton_clicked()] secKeys:" << m_securityMap.keys();
        return;
    }
    long tickerId = m_ibClient->getTickerId();
    Security* s = new Security(tickerId);
    m_securityMap[tickerId] = s;

    connect(s->getTimer(), SIGNAL(timeout()),
            this, SLOT(onPair1TimeOut()));

    Contract* c = s->contract();
//    c->conId = 0;
    c->symbol = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
    c->secType = m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText().toLocal8Bit();
    c->exchange = m_pair1ContractDetailsWidget->getUi()->exchangeLineEdit->text().toLocal8Bit();
    c->primaryExchange = m_pair1ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    c->currency = m_pair1ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();
    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText()=="FUT") {
        c->expiry = m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text().toLocal8Bit();
    }
    else if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == "OPT") {
        c->expiry = m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text().toLocal8Bit();
        c->strike = m_pair1ContractDetailsWidget->getUi()->strikeLineEdit->text().toDouble();
        c->right = m_pair1ContractDetailsWidget->getUi()->rightLineEdit->text().toLocal8Bit();
        c->multiplier = m_pair1ContractDetailsWidget->getUi()->multiplierLineEdit->text().toLocal8Bit();
    }

    long reqId = m_ibClient->getTickerId();
    m_contractDetailsMap[tickerId] = reqId;
    m_ibClient->reqContractDetails(reqId, *c);

    reqHistoricalData(tickerId);

    ui->pair2ShowButton->setEnabled(true);
}
void PairTabPage::on_pair2ShowButton_clicked()
{
    if (m_securityMap.keys().size() >= 2)
        return;
    long tickerId = m_ibClient->getTickerId();
    Security* pair2 = new Security(tickerId);
    m_securityMap[tickerId] = pair2;

    connect(pair2->getTimer(), SIGNAL(timeout()),
            this, SLOT(onPair2TimeOut()));

    ui->activateButton->setEnabled(true);
    ui->activateButton->setStyleSheet("background-color:green");

    Contract* contract = pair2->contract();
//    contract->conId = ui->pair2ContractIdLineEdit->text().toLong();
//    contract->conId = 0;
    contract->symbol = m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
    contract->secType = m_pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = m_pair2ContractDetailsWidget->getUi()->exchangeLineEdit->text().toLocal8Bit();
    contract->primaryExchange = m_pair2ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    contract->currency = m_pair2ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();

    long reqId = m_ibClient->getTickerId();
    m_contractDetailsMap[tickerId] = reqId;
    m_ibClient->reqContractDetails(reqId, *contract);

    reqHistoricalData(tickerId);
}

//void PairTabPage::on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1)
//{
//    m_pair1ContractDetailsWidget->getUi()->primaryExchangeLineEdit->setText(arg1.toUpper());
//}

//void PairTabPage::on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1)
//{
//    m_pair2ContractDetailsWidget->getUi()->primaryExchangeLineEdit->setText(arg1.toUpper());
//}

void PairTabPage::onPair1TimeOut()
{
    long sid = m_securityMap.keys().first();
    m_newBarMap[sid] = m_ibClient->getTickerId();
    reqHistoricalData(m_newBarMap[sid]);
}

void PairTabPage::onPair2TimeOut()
{
    long sid = m_securityMap.keys().last();
    m_newBarMap[sid] = m_ibClient->getTickerId();
    reqHistoricalData(m_newBarMap[sid]);
}

void PairTabPage::onActivateButtonClicked(bool)
{

    ui->activateButton->setEnabled(false);
    ui->deactivateButton->setEnabled(true);
    ui->deactivateButton->setStyleSheet("background-color:red");
    ui->activateButton->setAutoFillBackground(true);
    ui->activateButton->setPalette(ui->activateButton->parentWidget()->palette());

    for (int i=0;i<5;++i) {
        m_stdDevLayerPeaks.append(m_ratioStdDev.last());
    }

    // THIS IS ONLY FOR TESTING REMOVE
    placeOrder();

//    QTimer::singleShot(10000, this, SLOT(onSingleShotTimer()));

}

void PairTabPage::onDeactivateButtonClicked(bool)
{

}

void PairTabPage::onOrderStatus(long orderId, const QByteArray &status, int filled, int remaining, double avgFillPrice,
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
}

void PairTabPage::onOpenOrder(long orderId, const Contract &contract, const Order &order, const OrderState &orderState)
{
    qDebug() << "[DEBUG-onOpenOrder]"
             << orderId
             << contract.symbol
             << order.action
             << orderState.status;
}

void PairTabPage::onSingleShotTimer()
{
    qDebug() << "[DEBUG-onSingleShotTimer]";
//    m_ibClient->reqOpenOrders();
}

void PairTabPage::onContractDetails(int reqId, const ContractDetails &contractDetails)
{
//    qDebug() << "[DEBUG-onContractDetails]";
//             << reqId
//             << contractDetails.summary.symbol
//             << contractDetails.summary.localSymbol
//             << contractDetails.summary.conId;

    Security* s = m_securityMap[m_contractDetailsMap.key(reqId)];

    if (!s) {
        qDebug() << "[WARN-onContractDetails]"
                 << "m_securityMap.keys().size:" << m_securityMap.keys().size()
                 << "m_contractDetailsMap.keys().size:" << m_contractDetailsMap.keys().size();
        return;
    }

    s->setContractDetails(contractDetails);
//    Ui::ContractDetailsWidget* c = NULL;

////    Contract* ct = s->contract();

//    if (m_securityMap.values().indexOf(s) == 0)
//         c = m_pair1ContractDetailsWidget->getUi();
//    else if (m_securityMap.values().indexOf(s) == 1)
//        c = m_pair2ContractDetailsWidget->getUi();
//    else {
//        qDebug() << "[WARNING] Security not in securityMap";
//        return;
//    }

//    c->underCompLineEdit->setText(
//                QByteArray("conId=")
//                + QByteArray::number((int)contractDetails.summary.underComp->conId)
//                + QByteArray(";delta=")
//                + QByteArray::number((contractDetails.summary.underComp->delta))
//                + QByteArray(";price=")
//                + QByteArray::number((contractDetails.summary.underComp->price)));
//    c->comboLegsDescLineEdit->setText(ct->comboLegsDescrip);
//    c->securityIdLineEdit->setText(ct->secId);
//    c->secIdTypeLineEdit->setText(ct->secIdType);
//    c->includeExpiredLineEdit->setText(ct->includeExpired ? "true" : "false");
//    c->tradingClassLineEdit->setText(ct->tradingClass);
//    c->contractIdLineEdit->setText(QString::number(ct->conId));
//    c->localSymbolLineEdit->setText(ct->localSymbol);
//    c->primaryExchangeLineEdit->setText(ct->primaryExchange);
////    c->comboLegsLineEdit->setText(ct->comboLegs);
//    c->multiplierLineEdit->setText(ct->multiplier);
//    c->rightLineEdit->setText(ct->right);
//    c->strikeLineEdit->setText(QString::number(ct->strike));
//    c->expiryLineEdit->setText(ct->expiry);

}

void PairTabPage::onTradeEntryNumStdDevLayersChanged(int num)
{
    int numCurrentTabs = ui->layersTabWidget->count();

    if (num && !ui->waitCheckBox->isEnabled()) {
        ui->waitCheckBox->setEnabled(true);
        ui->layerBufferCheckBox->setEnabled(true);
        ui->layerBufferDoubleSpinBox->setEnabled(true);
    }
    else if (!num && ui->waitCheckBox->isEnabled()) {
        ui->waitCheckBox->setEnabled(false);
        ui->layerBufferCheckBox->setEnabled(false);
        ui->layerBufferDoubleSpinBox->setEnabled(false);
    }

    if (num > numCurrentTabs) {
        StdDevLayerTab* l = new StdDevLayerTab(numCurrentTabs, this);
        ui->layersTabWidget->addTab(l, QString::number(numCurrentTabs+1));
        connect(l->getUi()->layerTrailCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(onTrailCheckBoxStateChanged(int)));
//        l->getUi()->layerTrailCheckBox->setEnabled(false);
//        l->getUi()->layerTrailDoubleSpinBox->setEnabled(false);
    }
    else if (num < numCurrentTabs) {
        StdDevLayerTab* l = qobject_cast<StdDevLayerTab*>(ui->layersTabWidget->widget(ui->layersTabWidget->count()-1));
        ui->layersTabWidget->removeTab(ui->layersTabWidget->count()-1);
        delete l;

    }
}

void PairTabPage::onWaitCheckBoxStateChanged(int state)
{
    bool trailEnabled = true;

    if (state == Qt::Checked)
        trailEnabled = false;

    for (int i=0;i<ui->layersTabWidget->count();++i) {
        Ui::StdDevLayerTab* l = qobject_cast<StdDevLayerTab*>(ui->layersTabWidget->widget(i))->getUi();
        l->layerTrailCheckBox->setEnabled(trailEnabled);
        l->layerTrailDoubleSpinBox->setEnabled(trailEnabled);
    }
}

void PairTabPage::onTrailCheckBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->waitCheckBox->setEnabled(false);
        ui->layerBufferCheckBox->setEnabled(false);
        ui->layerBufferDoubleSpinBox->setEnabled(false);
    }
    else if (state == Qt::Unchecked) {
        ui->waitCheckBox->setEnabled(true);
        ui->layerBufferCheckBox->setEnabled(true);
        ui->layerBufferDoubleSpinBox->setEnabled(true);
    }
}

void PairTabPage::onCdui1SymbolTextChanged(QString text)
{
    qDebug() << "[DEBUG-onCdui1Sym]" << text;
    ui->pairsTabWidget->setTabText(0, text.toUpper());
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                text.toUpper() + "/" + ui->pairsTabWidget->tabText(1));
}

void PairTabPage::onCdui2SymbolTextChanged(QString text)
{
    ui->pairsTabWidget->setTabText(1, text.toUpper());
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                ui->pairsTabWidget->tabText(0) + "/" + text.toUpper());
}

ContractDetailsWidget *PairTabPage::getPair2ContractDetailsWidget() const
{
    return m_pair2ContractDetailsWidget;
}

ContractDetailsWidget *PairTabPage::getPair1ContractDetailsWidget() const
{
    return m_pair1ContractDetailsWidget;
}


void PairTabPage::reqHistoricalData(long tickerId)
{
    QDateTime dt = QDateTime::currentDateTime();

    TimeFrame tf = (TimeFrame)ui->timeFrameComboBox->currentIndex();
    QByteArray barSize = ui->timeFrameComboBox->currentText().toLocal8Bit();
    QByteArray durationStr;
    bool isNewBarReq = false;
    Security* security;

    if (!m_securityMap.keys().contains(tickerId)) {
        security = m_securityMap[m_newBarMap.key(tickerId)];
        isNewBarReq = true;
    }
    else {
        security = m_securityMap[tickerId];
    }

    /*
     *  390 mins in a trading day
     */

    switch (tf)
    {
    case SEC_1:
        m_timeFrame = SEC_1;
        m_timeFrameInSeconds = 1;
        if (!isNewBarReq)
            durationStr = "250 S";
        else
            durationStr = "50 S";
        break;
    case SEC_5:
        m_timeFrame = SEC_5;
        m_timeFrameInSeconds = 5;
        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 5) + " S";
        else
            durationStr = "50 S";
        break;
    case SEC_15:
        m_timeFrame = SEC_15;
        m_timeFrameInSeconds = 15;
        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 15) + " S";
        else
            durationStr = "50 S";
        break;
    case SEC_30:
        m_timeFrame = SEC_30;
        m_timeFrameInSeconds = 30;
        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 30) + " S";
        else
            durationStr = "50 S";
        break;
    case MIN_1:
        m_timeFrame = MIN_1;
        m_timeFrameInSeconds = 60;
        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "180 S";
        break;
    case MIN_2:
        m_timeFrame = MIN_2;
        m_timeFrameInSeconds = 120;
        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "360 S";
        break;
    case MIN_3:
        m_timeFrame = MIN_3;
        m_timeFrameInSeconds = 180;
        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "540 S";
        break;
    case MIN_5:
        m_timeFrame = MIN_5;
        m_timeFrameInSeconds = 60 * 5;
        if (!isNewBarReq)
            durationStr = "4 D";
        else
            durationStr = "540 S";
        break;
    case MIN_15:
        m_timeFrame = MIN_15;
        m_timeFrameInSeconds = 60 * 15;
        if (!isNewBarReq)
            durationStr = "11 D";
        else
            durationStr = "1 D";
        break;
    case MIN_30:
        m_timeFrame = MIN_30;
        m_timeFrameInSeconds = 60 * 30;
        if (!isNewBarReq)
            durationStr = "21 D";
        else
            durationStr = "1 D";
        break;
    case HOUR_1:
        m_timeFrame = HOUR_1;
        m_timeFrameInSeconds = 60 * 60;
        if (!isNewBarReq)
            durationStr = "1 M";            // FIXME: I can only get 140 data points for 1 hour.. need to call again ?
        else
            durationStr = "1 D";
        break;
    case DAY_1:
        m_timeFrame = DAY_1;
        m_timeFrameInSeconds = 60 * 60 * 24;
        if (!isNewBarReq)
            durationStr = "1 Y";
        else
            durationStr = "5 D";
        break;
    case DAY_1 + 1:
        // handle 1 week time frame specially
        break;
    }

    m_ibClient->reqHistoricalData(tickerId
                                  , *(security->contract())
                                  , dt.toUTC().toString("yyyyMMdd hh:mm:ss 'GMT'").toLocal8Bit()
                                  , durationStr
                                  , barSize
                                  , "TRADES"
                                  , 1
                                  , 2
                                  , QList<TagValue*>());
}

void PairTabPage::placeOrders()
{
    // TODO: NOT NEEDED.. SEE placeOrder

    Security* s1 = m_securityMap.values().first();
    Security* s2 = m_securityMap.values().last();

    Order* o1 = s1->getOrder();
    Order* o2 = s2->getOrder();

    long oid1 = m_ibClient->getOrderId();
    long oid2 = m_ibClient->getOrderId();

    s1->setOrderId(oid1);
    s2->setOrderId(oid2);

    o1->action = "BUY";
    o1->totalQuantity = 1000;
    o1->orderType = "LMT";
    o1->lmtPrice = 0.01;
    o1->account = "DU210787";

    o2->action = "SSHORT";
    o2->totalQuantity = 50;
    o2->orderType = "MKT";
    o2->account = "DU210787";

    qDebug() << "PLACING ORDERS NOW";

    m_ibClient->placeOrder(oid1, *(s1->contract()), *(o1));
    //    m_ibClient->placeOrder(oid2, *(s2->contract()), *(o2));
}

void PairTabPage::placeOrder()
{
    qDebug() << "[DEBUG-placeOrder]";
    ComboLeg c1;
    ComboLeg c2;

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    c1.conId = s1->contract()->conId;
    c1.ratio = 1;
    c1.action = "SELL";
    c1.exchange = "SMART";
    c1.openClose = 0;
    c1.shortSaleSlot = 0;
    c1.designatedLocation = "";

    c2.conId = s2->contract()->conId;
    c2.ratio = 1;
    c2.action = "BUY";
    c2.exchange = "SMART";
    c2.openClose = 0;
    c2.shortSaleSlot = 0;
    c2.designatedLocation = "";

    QList<ComboLeg*> clist;
    clist.append(&c1);
    clist.append(&c2);

    Contract ct1 = *(s1->contract());

    ct1.symbol = "USD";
    ct1.secType = "BAG";
    ct1.comboLegs = clist;

    Order order;
    order.action = "BUY";
    order.totalQuantity = 1;
    order.orderType = "MKT";

    long id = m_ibClient->getOrderId();
    m_ibClient->placeOrder(id, ct1, order);

}

void PairTabPage::exitOrder()
{

}

void PairTabPage::showPlot(long tickerId, ChartType chartType)
{
    Q_UNUSED(chartType);

    Security* s = m_securityMap[tickerId];
    DataVecsHist* dvh = s->getHistData(m_timeFrame);

    qDebug() << "[DEBUG-showPlot] last dt:" << QDateTime::fromTime_t(dvh->timeStamp.last()).toString("yy.MM.dd/hh:mm:ss")
             << "numbars:" << dvh->timeStamp.size();

//    for (int i=0;i<dvh->timeStamp.size();++i) {
//        qDebug() << i+1 << ")"
//                << tickerId
//                << QDateTime::fromTime_t(dvh->timeStamp.at(i)).toString("YY-MM-dd/hh:mm:ss")
//                << dvh->open.at(i)
//                << dvh->high.at(i)
//                << dvh->low.at(i)
//                << dvh->close.at(i)
//                << dvh->barCount.at(i)
//                << dvh->wap.at(i)
//                << dvh->hasGaps.at(i);
//    }

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, s->contract()->symbol);
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));
    g->setData(dvh->timeStamp, dvh->close);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(dvh->timeStamp.first(), dvh->timeStamp.last());

    double min = getMin(dvh->close);
    double max = getMax(dvh->close);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::appendPlot(long tickerId)
{
    Security* s = m_securityMap[tickerId];
    DataVecsHist* dvh = s->getHistData(m_timeFrame);


    int ilbts = dvh->timeStamp.indexOf(s->lastBarsTimeStamp());

    for (int i=ilbts+1;i<dvh->timeStamp.size();++i) {
        qDebug() << "[APPEND]"
                 << i << ")"
                << tickerId
                << QDateTime::fromTime_t(dvh->timeStamp.at(i)).toString("YY-MM-dd/hh:mm:ss")
                << dvh->open.at(i)
                << dvh->high.at(i)
                << dvh->low.at(i)
                << dvh->close.at(i)
                << dvh->barCount.at(i)
                << dvh->wap.at(i)
                << dvh->hasGaps.at(i);
    }

    s->setLastBarsTimeStamp(dvh->timeStamp.last());
}

void PairTabPage::plotRatio()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    if (m_ratio.isEmpty()) {
//        QVector<double>* c1 = &dvh1->close;
//        QVector<double>* c2 = &dvh2->close;

//        int size = 0;

//        if (c1->size() < c2->size())
//            size = c1->size();
//        else if (c1->size() > c2->size())
//            size = c2->size();
//        else
//            size = c1->size();

//        m_ratio = getRatio(c1->mid(c1->size()-size), c2->mid(c2->size()-size));
        m_ratio = getRatio(dvh1->close, dvh2->close);
    }

//    qDebug() << "[DEBUG-plotRatio] m_ratio.size:" << m_ratio.size() << "dvh1->timeStamp.size:" << dvh1->timeStamp.size();
//    qDebug() << "[DEBUG-plotRatio]" << dvh1->timeStamp;


    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "Ratio");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(dvh1->timeStamp, m_ratio);


    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(dvh1->timeStamp.first(), dvh1->timeStamp.last());

    double min = getMin(m_ratio);
    double max = getMax(m_ratio);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();

}

void PairTabPage::plotRatioMA()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
//    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

//    if (m_ratio.isEmpty()) {
//        QVector<double>* c1 = &dvh1->close;
//        QVector<double>* c2 = &dvh2->close;

//        int size = 0;

//        if (c1->size() < c2->size())
//            size = c1->size();
//        else if (c1->size() > c2->size())
//            size = c2->size();
//        else
//            size = c1->size();

//        m_ratio = getRatio(c1->mid(c1->size()-size), c2->mid(c2->size()-size));
//    }

//    qDebug() << "[DEBUG-plotRatioMA]" << m_ratio;
//    qDebug() << "[DEBUG-plotRatioMA]" << dvh1->timeStamp;

    m_ratioMA = getMA(m_ratio, ui->maPeriodSpinBox->value());

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "RatIo MA");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    QVector<double> ts = dvh1->timeStamp.mid(dvh1->timeStamp.size()-m_ratioMA.size());

    g->setData(ts, m_ratioMA);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(ts.first(), ts.last());

    double min = getMin(m_ratioMA);
    double max = getMax(m_ratioMA);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::plotRatioStdDev()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    if (m_ratio.isEmpty()) {
        QVector<double>* c1 = &dvh1->close;
        QVector<double>* c2 = &dvh2->close;

        int size = 0;

        if (c1->size() < c2->size())
            size = c1->size();
        else if (c1->size() > c2->size())
            size = c2->size();
        else
            size = c1->size();

        m_ratio = getRatio(c1->mid(c1->size()-size), c2->mid(c2->size()-size));
    }

//    qDebug() << "[DEBUG-plotRatioMA]" << m_ratio;
//    qDebug() << "[DEBUG-plotRatioMA]" << dvh1->timeStamp;

    m_ratioStdDev = getMovingStdDev(m_ratio, ui->stdDevPeriodSpinBox->value());

//    qDebug() << "[DEBUG-plotStdDevOfRatio] stddev:" << m_ratioStdDev;

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "StdDev of Mean");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(dvh1->timeStamp.mid(dvh1->timeStamp.size() - m_ratioStdDev.size()), m_ratioStdDev);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(dvh1->timeStamp.first(), dvh1->timeStamp.last());

    double min = getMin(m_ratioStdDev);
    double max = getMax(m_ratioStdDev);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::plotRatioPercentFromMean()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioPercentFromMean = getPercentFromMean(m_ratio);

//    qDebug() << m_ratioPercentFromMean;

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "Percent From Mean");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(dvh1->timeStamp.mid(dvh1->timeStamp.size() - m_ratioPercentFromMean.size()), m_ratioPercentFromMean);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(dvh1->timeStamp.first(), dvh1->timeStamp.last());

    double min = getMin(m_ratioPercentFromMean);
    double max = getMax(m_ratioPercentFromMean);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::plotCorrelation()
{
    DataVecsHist* d1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* d2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    m_correlation = getCorrelation(d1->close, d2->close);

//    qDebug() << m_correlation;

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "Correlation");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(d1->timeStamp.mid(d1->timeStamp.size() - m_correlation.size()), m_correlation);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(d1->timeStamp.first(), d1->timeStamp.last());

    double min = getMin(m_correlation);
    double max = getMax(m_correlation);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::plotCointegration()
{
    for (int i=0;i<250;++i) {
        m_cointegration[i] = 0;
    }
}

void PairTabPage::plotRatioVolatility()
{
    DataVecsHist* d1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioVolatility = getVolatility(m_ratio, ui->volatilityPeriodSpinBox->value());

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "Ratio Volatility");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(d1->timeStamp.mid(d1->timeStamp.size() - m_ratioVolatility.size()), m_ratioVolatility);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(d1->timeStamp.first(), d1->timeStamp.last());

    double min = getMin(m_ratioVolatility);
    double max = getMax(m_ratioVolatility);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();
}

void PairTabPage::plotRatioRSI()
{
    DataVecsHist* d1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioRSI = getRSI(m_ratio, ui->rsiPeriodSpinBox->value());

//    qDebug() << "[DEBUG-plotRatioRSI]" << m_ratioRSI;

    QCustomPlot* cp = new QCustomPlot();

    int tabIdx = ui->tabWidget->addTab(cp, "Ratio RSI");
    ui->tabWidget->setCurrentIndex(tabIdx);
    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

    cp->addGraph();
    QCPGraph* g = cp->graph(0);
    g->setPen(QPen(Qt::blue));
    g->setBrush(QBrush(QColor(0, 0, 255, 20)));

    g->setData(d1->timeStamp.mid(d1->timeStamp.size() - m_ratioRSI.size()), m_ratioRSI);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    cp->xAxis->setDateTimeFormat("yy.MM.dd\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(d1->timeStamp.at(d1->timeStamp.size() - m_ratioRSI.size()), d1->timeStamp.last());

//    double min = getMin(m_ratioRSI);
//    double max = getMax(m_ratioRSI);

    cp->yAxis->setRange(0, 100);

    cp->replot();
}

void PairTabPage::plotSpreadRSI()
{

}

void PairTabPage::addTableRow()
{
    QTableWidget* tab = mwui->homeTableWidget;

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    DataVecsHist* d1 = s1->getHistData(m_timeFrame);
    DataVecsHist* d2 = s2->getHistData(m_timeFrame);

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
            << "Ratio RSI"
            << "Spread RSI";

    QStringList itemList;
    itemList << m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text()
             << m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text()
             << QString::number(d1->close.last())
             << QString::number(d2->close.last())
             << QString::number(m_ratio.last())
             << QString::number(m_ratioStdDev.last())
             << QString::number(m_ratioPercentFromMean.last())
             << QString::number(m_correlation.last())
////             << QString::number(m_cointegration.last())
             << QString::number(-99)
             << QString::number(m_ratioVolatility.last())
             << QString::number(m_ratioRSI.last())
////             << QString::number(m_spreadRSI.last())
                ;

//    qDebug() << "itemList:" << itemList;
    tab->setRowCount(tab->rowCount()+1);
    for (int i=0;i<itemList.size();++i) {
        QTableWidgetItem* item = new QTableWidgetItem(itemList.at(i));
        tab->setItem(tab->rowCount()-1, i, item);
    }
    tab->repaint();

}

void PairTabPage::checkTradeTriggers()
{
    if (!m_ratioRSITriggerActivated && ui->tradeEntryRSICheckBox->checkState() == Qt::Checked) {
        // NOTE: tradeEntryRSILowerSpinBox is not needed on ratio and entries... maybe exits
        double lastRSI = m_ratioRSI.last();
        if (lastRSI > ui->tradeEntryRSIUpperSpinBox->value()) {
            placeOrder();
        }
    }

    if (!m_percentFromMeanTriggerActivated && ui->tradeEntryPercentFromMeanCheckBox->checkState() == Qt::Checked) {
        double lastPofM = m_ratioPercentFromMean.last();
        if (lastPofM > ui->tradeEntryPercentFromMeanSpinBox->value()) {
            placeOrder();
        }
    }

    if (ui->tradeEntryNumStdDevLayersSpinBox->value() > 0) {

        int numLayers = ui->tradeEntryNumStdDevLayersSpinBox->value();
        double lastStdDev = m_ratioStdDev.last();
        bool wait = false;

        for (int i=0;i<numLayers;++i) {
            if (lastStdDev > m_stdDevLayerPeaks.at(i))
                m_stdDevLayerPeaks[i] = lastStdDev;
            StdDevLayerTab* t = qobject_cast<StdDevLayerTab*>(ui->layersTabWidget->widget(i));
            double stdDevVal = t->findChild<QDoubleSpinBox*>("layerStdDevDoubleSpinBox")->value();
            double peak = m_stdDevLayerPeaks.at(i);
            double buffVal = 0;
            double trailVal = 0;
            if (t->findChild<QCheckBox*>("layerBufferCheckBox")->checkState() == Qt::Checked) {
                buffVal = t->findChild<QDoubleSpinBox*>("layerBufferDoubleSpinBox")->value();
            }

            if (t->findChild<QCheckBox*>("layerTrailCheckBox")->checkState() == Qt::Checked) {
                trailVal = t->findChild<QDoubleSpinBox*>("layerTrailDoubleSpinBox")->value();
            }

            if (!trailVal && t->findChild<QCheckBox*>("waitCheckBox")->checkState() == Qt::Checked) {
                wait = true;
            }

            // Now check the strategies

            if (trailVal) {
                if (t->findChild<QCheckBox*>("layerStdMinCheckBox")->checkState() == Qt::Checked) {
                    double stdMin = t->findChild<QDoubleSpinBox*>("layerStdMinDoubleSpinBox")->value();
                    if (peak > stdMin && lastStdDev < (peak - trailVal)) {
                        placeOrder();
                    }
                }
                else if (lastStdDev < (peak - trailVal)) {
                    placeOrder();
                }
            }
            if (wait) {
                if (i == (numLayers - 1)
                         && ui->waitCheckBox->checkState() == Qt::Checked
                         && peak > stdDevVal
                         && lastStdDev < stdDevVal - buffVal) // if bufVal is zero its like its not being used
                {
                    for (int i=0;i<numLayers;++i)
                        placeOrder();
                }
            }

            if (!wait && !trailVal && lastStdDev > stdDevVal)
                placeOrder();
        }
    }
}

void PairTabPage::checkTradeExits()
{
    if (ui->tradeExitPercentStopLossCheckBox->checkState() == Qt::Checked) {

        // TODO: AFTER ORDERS AND PRICE OF ORDER IS DONE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }

    if (ui->tradeExitPercentFromMeanCheckBox->checkState() == Qt::Checked) {
        double lastPercentFromMean = m_ratioPercentFromMean.last();
        double trigger = ui->tradeExitPercentFromMeanSpinBox->value();
        if (lastPercentFromMean < trigger)
            exitOrder();

    }

    if (ui->tradeExitStdDevCheckBox->checkState() == Qt::Checked) {
        double lastStdDev = m_ratioStdDev.last();
        double trigger = ui->tradeExitStdDevDoubleSpinBox->value();
        if (lastStdDev < trigger)
            exitOrder();
    }
}
















































