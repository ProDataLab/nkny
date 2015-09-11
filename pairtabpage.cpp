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


PairTabPage::PairTabPage(IBClient *ibClient, const QStringList & managedAccounts, QWidget *parent)
    : QWidget(parent)
    , m_ibClient(ibClient)
    , m_managedAccounts(managedAccounts)
    , ui(new Ui::PairTabPage)
    , m_gettingMoreHistoricalData(false)
    , m_homeTablePageRowIndex(-1)
    , m_bothPairsUpdated(true)
{

//    qDebug() << "[DEBUG-PairTabPage]";

    ui->setupUi(this);

    m_mainWindow = qobject_cast<MainWindow*>(parent);
    mwui = m_mainWindow->getUi();

    m_origButtonStyleSheet = ui->activateButton->styleSheet();

    QStringList secTypes;
    secTypes += "STK";
    secTypes += "FUT";
//    secTypes += "OPT";
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


    cdui1->symbolLineEdit->setText("MSFT");
    cdui1->primaryExchangeLineEdit->setText("NYSE");

    cdui2->symbolLineEdit->setText("AAPL");
    cdui2->primaryExchangeLineEdit->setText("NASDAQ");

    ui->pairsTabWidget->tabBar()->setTabText(0, cdui1->symbolLineEdit->text());
    ui->pairsTabWidget->tabBar()->setTabText(1, cdui2->symbolLineEdit->text());

    ui->managedAccountsComboBox->addItems(m_managedAccounts);

    qDebug() << "[DEBUG-PairTabPage]" << ui->pair1ShowButton->styleSheet();
//    ui->pair1ShowButton->setStyleSheet("background-color:green");
    ui->pair2ShowButton->setEnabled(false);


    connect(cdui1->symbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui1SymbolTextChanged(QString)));
    connect(cdui2->symbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui2SymbolTextChanged(QString)));

    connect(cdui1->localSymbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui1SymbolTextChanged(QString)));
    connect(cdui2->localSymbolLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onCdui2SymbolTextChanged(QString)));

    connect(ui->activateButton, SIGNAL(clicked(bool)),
            this, SLOT(onActivateButtonClicked(bool)));
    connect(ui->deactivateButton, SIGNAL(clicked(bool)),
            this, SLOT(onDeactivateButtonClicked(bool)));
//    connect(m_ibClient, SIGNAL(orderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)),
//            this, SLOT(onOrderStatus(long,QByteArray,int,int,double,int,int,double,int,QByteArray)));
//    connect(m_ibClient, SIGNAL(openOrder(long,Contract,Order,OrderState)),
//            this, SLOT(onOpenOrder(long,Contract,Order,OrderState)));
    connect(m_ibClient, SIGNAL(contractDetails(int,ContractDetails)),
            this, SLOT(onContractDetails(int,ContractDetails)));
    connect(m_ibClient, SIGNAL(historicalData(long,QByteArray,double,double,double,double,int,int,double,int)),
            this, SLOT(onHistoricalData(long,QByteArray,double,double,double,double,int,int,double,int)));
    connect(ui->tradeEntryNumStdDevLayersSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(onTradeEntryNumStdDevLayersChanged(int)));
    connect(ui->waitCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(onWaitCheckBoxStateChanged(int)));
    connect(m_ibClient, SIGNAL(contractDetailsEnd(int)),
            this, SLOT(onContractDetailsEnd(int)));
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
//    qDebug() << "[DEBUG-onHistoricalData]";

    if (!(m_securityMap.keys().contains(reqId)
          || m_newBarMap.keys().contains(reqId)
          || m_moreDataMap.keys().contains(reqId)))
        return;

    Security* s= NULL;
    long sid = 0;
    double timeStamp = 0;
    bool isNewBarReq = false;
    bool isMoreDataReq = false;

    if (m_timeFrame == DAY_1)
        timeStamp = (double)QDateTime::fromString(date, "yyyyMMdd").toTime_t();
    else
        timeStamp = date.toDouble();


    if (m_securityMap.keys().contains(reqId)) {
        s = m_securityMap[reqId];
        sid = reqId;
//        qDebug() << "[DEBUG-onHistoricalData] initial data request";
    }
    else if (m_newBarMap.values().contains(reqId)) {
        isNewBarReq = true;
        sid = m_newBarMap.key(reqId);
        s = m_securityMap[sid];
//        qDebug() << "[DEBUG-onHistoricalData] new bar data request";
    }
    else if (m_moreDataMap.values().contains(reqId)) {
        isMoreDataReq = true;
        sid = m_moreDataMap.key(reqId);
        s = m_securityMap[sid];
//        qDebug() << "[DEBUG-onHistoricalData] more data request";
    }
    else {
        qDebug() << "[ERROR-onHistoricalData] request Id UNKNOWN";
        qDebug() << "    secMapKeys:" << m_securityMap.keys();
        qDebug() << "    secMapVals:" << m_securityMap.values();
        qDebug() << "    reqId:" << reqId;
        return;
    }

    if (!isNewBarReq) {
        if (date.startsWith("finished")) {
            if (!isMoreDataReq) {
                double lastBarsTimeStamp = s->getHistData(m_timeFrame)->timeStamp.last();
                s->setLastBarsTimeStamp(lastBarsTimeStamp);

                s->getTimer()->start(m_timeFrameInSeconds * 1000);

                DataVecsHist* dvh = s->getHistData(m_timeFrame);

                qDebug() << "[DEBUG-onHistoricalData] NUM BARS RECEIVED:" << dvh->timeStamp.size();

                if (dvh->timeStamp.size() >= 250) {
                    s->fixHistDataSize(m_timeFrame);
                }
                else {
                    // FIXME: I need more bars (1 HOUR BARS)
//                    m_moreDataMap[s->tickerId()] = m_ibClient->getTickerId();
//                    QTimer::singleShot(1000, this, SLOT(onMoreHistoricalDataNeeded()));
//                    return;
                }
            }
            else {
//                qDebug() << "[DEBUG-onHistoricalData] firstTimeStamp:"  << QDateTime::fromTime_t((uint)s->getHistData(m_timeFrame)->timeStamp.first()).toString("yyyyMMdd/hh:mm:ss");
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
            if (isMoreDataReq) {
//                qDebug() << "[DEBUG-onHistoricalData]" << m_timeFrame << QDateTime::fromTime_t((int)timeStamp).toString("yyyyMMdd/hh:mm:ss") << open << high << low << close << volume << barCount << WAP << hasGaps;

            }
            else {
//                qDebug() << "[DEBUG-onHistoricalData]" << m_timeFrame << QDateTime::fromTime_t((int)timeStamp).toString("yyyyMMdd/hh:mm:ss") << open << high << low << close << volume << barCount << WAP << hasGaps;
                s->appendHistData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
            }
        }
    }
    else if (isNewBarReq){
        if (date.startsWith("finished")) {
            s->handleNewBarData(m_timeFrame);
            if (ui->activateButton->isEnabled()) {
                checkTradeTriggers();
                if (!s->getSecurityOrderMap().isEmpty())
                    checkTradeExits();
            }
            appendPlotsAndTable(sid);
        }
        else {
            s->appendNewBarData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
        }
    }
//    qDebug() << "[DEBUG-onHistoricalData] leaving";
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

    qDebug() << "[DEBUG-on_pair1ShowButtonClicked] m_securityMap.size():" << m_securityMap.size();

    connect(s->getTimer(), SIGNAL(timeout()),
            this, SLOT(onPair1TimeOut()));

    Contract* c = s->contract();
//    c->conId = 0;
    c->secType = m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText().toLocal8Bit();
    c->exchange = m_pair1ContractDetailsWidget->getUi()->exchangeLineEdit->text().toLocal8Bit();
    c->primaryExchange = m_pair1ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    c->currency = m_pair1ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();
    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == "STK") {
        c->symbol = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
    }
    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText()=="FUT") {
        c->symbol = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
        c->expiry = m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text().toLocal8Bit();
        c->localSymbol = m_pair1ContractDetailsWidget->getUi()->localSymbolLineEdit->text().toLocal8Bit();
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

    ui->pair2ShowButton->setEnabled(true);
    ui->pair1ShowButton->setEnabled(false);

    ui->timeFrameComboBox->setEnabled(false);
}
void PairTabPage::on_pair2ShowButton_clicked()
{
    ui->pair2ShowButton->setEnabled(false);

    if (m_securityMap.keys().size() >= 2)
        return;
    long tickerId = m_ibClient->getTickerId();
    Security* pair2 = new Security(tickerId);
    m_securityMap[tickerId] = pair2;

    qDebug() << "[DEBUG-on_pair2ShowButtonClicked] m_securityMap.size():" << m_securityMap.size();

    m_tabSymbol = ui->pairsTabWidget->tabText(0) + "/" + ui->pairsTabWidget->tabText(1) + " (" + m_timeFrameString + ")";
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                m_tabSymbol);


    connect(pair2->getTimer(), SIGNAL(timeout()),
            this, SLOT(onPair2TimeOut()));

    ui->activateButton->setEnabled(true);
//    ui->activateButton->setStyleSheet("background-color:green");

    Contract* contract = pair2->contract();
//    contract->conId = ui->pair2ContractIdLineEdit->text().toLong();
//    contract->conId = 0;
    contract->symbol = m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
    contract->secType = m_pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = m_pair2ContractDetailsWidget->getUi()->exchangeLineEdit->text().toLocal8Bit();
    contract->primaryExchange = m_pair2ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    contract->currency = m_pair2ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();
    contract->expiry = m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text().toLocal8Bit();
    contract->localSymbol = m_pair1ContractDetailsWidget->getUi()->localSymbolLineEdit->text().toLocal8Bit();
    contract->strike = m_pair1ContractDetailsWidget->getUi()->strikeLineEdit->text().toDouble();
    contract->right = m_pair1ContractDetailsWidget->getUi()->rightLineEdit->text().toLocal8Bit();
    contract->multiplier = m_pair1ContractDetailsWidget->getUi()->multiplierLineEdit->text().toLocal8Bit();

    long reqId = m_ibClient->getTickerId();
    m_contractDetailsMap[tickerId] = reqId;
    m_ibClient->reqContractDetails(reqId, *contract);


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
//    ui->deactivateButton->setStyleSheet("background-color:red");
//    ui->activateButton->setAutoFillBackground(true);
//    ui->activateButton->setPalette(ui->activateButton->parentWidget()->palette());

    for (int i=0;i<5;++i) {
        m_stdDevLayerPeaks.append(m_ratioStdDev.last());
    }

    // THIS IS ONLY FOR TESTING REMOVE
    placeOrder();

//    QTimer::singleShot(10000, this, SLOT(onSingleShotTimer()));

}

void PairTabPage::onDeactivateButtonClicked(bool)
{
    m_ibClient->reqOpenOrders();
}



void PairTabPage::onSingleShotTimer()
{
    qDebug() << "[DEBUG-onSingleShotTimer]";
//    m_ibClient->reqOpenOrders();
}

void PairTabPage::onContractDetails(int reqId, const ContractDetails &contractDetails)
{
    qDebug() << "[DEBUG-onContractDetails]"
             << reqId
             << contractDetails.summary.symbol
             << contractDetails.summary.localSymbol
             << contractDetails.summary.conId
             << contractDetails.summary.expiry;

    Security* s = m_securityMap[m_contractDetailsMap.key(reqId)];

    if (!s) {
//        qDebug() << "[WARN-onContractDetails]"
//                 << "m_securityMap.keys().size:" << m_securityMap.keys().size()
//                 << "m_contractDetailsMap.keys().size:" << m_contractDetailsMap.keys().size();
        return;
    }

    s->setContractDetails(contractDetails);

    reqHistoricalData(m_contractDetailsMap.key(reqId));


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

void PairTabPage::onContractDetailsEnd(int reqId)
{
    Q_UNUSED(reqId);
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
//    qDebug() << "[DEBUG-onCdui1Sym]" << text;
    ui->pairsTabWidget->setTabText(0, text.toUpper());
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                text.toUpper() + "/" + ui->pairsTabWidget->tabText(1));
}

void PairTabPage::onCdui2SymbolTextChanged(QString text)
{
    ui->pairsTabWidget->setTabText(1, text.toUpper());
    m_tabSymbol = ui->pairsTabWidget->tabText(0) + "/" + text.toUpper() + "(" + m_timeFrameString + ")";
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                m_tabSymbol);
}



ContractDetailsWidget *PairTabPage::getPair2ContractDetailsWidget() const
{
    return m_pair2ContractDetailsWidget;
}

bool PairTabPage::reqClosePair()
{
    qDebug() << "[DEBUG-reqClosePair]";

    Security* s1=NULL;
    Security* s2=NULL;

    // THIS IS A HACK FOR AN BAD VALUE AT MAP INDEX 0... FIXME !!!;
    if (m_securityMap.size() == 1)
        s1 = m_securityMap.values().at(0);
    if (m_securityMap.keys().size() == 2) {
        s1 = m_securityMap.values().at(0);
        s2 = m_securityMap.values().at(1);
    }
    else if (m_securityMap.keys().size() == 3) {
        s1 = m_securityMap.values().at(1);
        s2 = m_securityMap.values().at(2);
    }

    QSettings s;

    // IF S2 NOT SET YET
    if (m_securityMap.isEmpty() || s2 == NULL) {
        s.remove(m_tabSymbol);
        return true;
    }
    if (s1->getSecurityOrderMap().isEmpty()
            && s2->getSecurityOrderMap().isEmpty()
            && mwui->homeTableWidget->rowCount() > 0
            && m_homeTablePageRowIndex != -1) {
//        qDebug() << "[DEBUG-reqClosePair] rowIdx:" << m_homeTablePageRowIndex;
//        qDebug() << "[DEBUG-reqClosePair] rowCount:" << mwui->homeTableWidget->rowCount();
        QTableWidget* tw = mwui->homeTableWidget;
        for (int r=0;r<tw->rowCount();++r) {
            if (tw->item(r,0)->text() == s1->contract()->symbol
                    && tw->item(r,1)->text() == s2->contract()->symbol) {
                tw->removeRow(r);
                tw->repaint();
            }
        }
        s.remove(m_tabSymbol);
        return true;
    }
    else
        return false;
}

ContractDetailsWidget *PairTabPage::getPair1ContractDetailsWidget() const
{
    return m_pair1ContractDetailsWidget;
}


void PairTabPage::reqHistoricalData(long tickerId, QDateTime dt)
{
    qDebug() << "[DEBUG-reqHistoricalData]";

    TimeFrame tf = (TimeFrame)ui->timeFrameComboBox->currentIndex();
    QByteArray barSize = ui->timeFrameComboBox->currentText().toLocal8Bit();
    QByteArray durationStr;
    bool isNewBarReq = false;
    Security* security;

    if (m_newBarMap.values().contains(tickerId)) {
        security = m_securityMap[m_newBarMap.key(tickerId)];
        isNewBarReq = true;
    }
    else if (m_securityMap.keys().contains(tickerId)) {
        security = m_securityMap[tickerId];
    }
    else if (m_moreDataMap.values().contains(tickerId)) {
        security = m_securityMap[m_moreDataMap.key(tickerId)];
    }
    else {
        qFatal("Security* security.. was not established!!");
    }

    /*
     *  390 mins in a trading day
     */


    switch (tf)
    {
    case SEC_1:
        m_timeFrame = SEC_1;
        m_timeFrameInSeconds = 1;
        m_timeFrameString = "1 Sec";

        if (!isNewBarReq)
            durationStr = "250 S";
        else
            durationStr = "50 S";
        break;
    case SEC_5:
        m_timeFrame = SEC_5;
        m_timeFrameInSeconds = 5;
        m_timeFrameString = "5 Sec";

        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 5) + " S";
        else
            durationStr = "50 S";
        break;
    case SEC_15:
        m_timeFrame = SEC_15;
        m_timeFrameInSeconds = 15;
        m_timeFrameString = "15 Sec";

        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 15) + " S";
        else
            durationStr = "50 S";
        break;
    case SEC_30:
        m_timeFrame = SEC_30;
        m_timeFrameInSeconds = 30;
        m_timeFrameString = "30 Sec";

        if (!isNewBarReq)
            durationStr = QByteArray::number((250) * 30) + " S";
        else
            durationStr = "50 S";
        break;
    case MIN_1:
        m_timeFrame = MIN_1;
        m_timeFrameInSeconds = 60;
        m_timeFrameString = "1 Min";

        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "180 S";
        break;
    case MIN_2:
        m_timeFrame = MIN_2;
        m_timeFrameInSeconds = 120;
        m_timeFrameString = "2 Min";

        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "360 S";
        break;
    case MIN_3:
        m_timeFrame = MIN_3;
        m_timeFrameInSeconds = 180;
        m_timeFrameString = "3 Min";

        if (!isNewBarReq)
            durationStr = "2 D";
        else
            durationStr = "540 S";
        break;
    case MIN_5:
        m_timeFrame = MIN_5;
        m_timeFrameInSeconds = 60 * 5;
        m_timeFrameString = "5 Min";

        if (!isNewBarReq)
            durationStr = "4 D";
        else
            durationStr = "540 S";
        break;
    case MIN_15:
        m_timeFrame = MIN_15;
        m_timeFrameInSeconds = 60 * 15;
        m_timeFrameString = "15 Min";

        if (!isNewBarReq)
            durationStr = "11 D";
        else
            durationStr = "1 D";
        break;
    case MIN_30:
        m_timeFrame = MIN_30;
        m_timeFrameInSeconds = 60 * 30;
        m_timeFrameString = "30 Min";

        if (!isNewBarReq)
            durationStr = "21 D";
        else
            durationStr = "1 D";
        break;
    case HOUR_1:
        m_timeFrame = HOUR_1;
        m_timeFrameInSeconds = 60 * 60;
        m_timeFrameString = "1 Hour";

        if (!isNewBarReq)
            durationStr = "1 M";            // FIXME: I can only get 140 data points for 1 hour.. need to call again ?
        else
            durationStr = "1 D";
        break;
    case DAY_1:
        m_timeFrame = DAY_1;
        m_timeFrameInSeconds = 60 * 60 * 24;
        m_timeFrameString = "1 Day";

        if (!isNewBarReq)
            durationStr = "1 Y";
        else
            durationStr = "5 D";
        break;
    case DAY_1 + 1:
        // handle 1 week time frame specially
        break;
    }
//    qDebug() << "[DEBUG-reqHistoricalData] -secs:" << m_timeFrameInSeconds;


//    qDebug() << "[DEBUG-reqHistoricalData] dt1:" << dt.toString("yyyyMMdd/hh:mm:ss");

    if (m_timeFrame == DAY_1) {
        dt = dt.addDays(-1);
    }
    else {
        QTime t(dt.time());
        t = t.addSecs(-m_timeFrameInSeconds);
        dt.setTime(t);
    }

//    qDebug() << "[DEBUG-reqHistoricalData] dt2:" << dt.toString("yyyyMMdd/hh:mm:ss");

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

void PairTabPage::onMoreHistoricalDataNeeded()
{
    qDebug() << "[DEBUG-onMoreHistoricalDataNeeded]";
    m_gettingMoreHistoricalData = true;
    Security* s = m_securityMap[m_moreDataMap.keys().last()];
    double firstTimeStamp = s->getHistData(m_timeFrame)->timeStamp.first();
    QDateTime firstDT = QDateTime::fromTime_t((uint)firstTimeStamp);
    long id = m_moreDataMap[m_moreDataMap.values().last()];
    reqHistoricalData(id, firstDT);
}
void PairTabPage::setTabSymbol(const QString &tabSymbol)
{
    m_tabSymbol = tabSymbol;
}

void PairTabPage::setIbClient(IBClient *ibClient)
{
    m_ibClient = ibClient;
}

QString PairTabPage::getTimeFrameString() const
{
    return m_timeFrameString;
}

void PairTabPage::writeSettings() const
{
    QSettings s;
    s.beginGroup(m_tabSymbol);
    s.setValue("activateButtonEnabled", ui->activateButton->isEnabled());
    s.setValue("dectivateButtonEnabled", ui->deactivateButton->isEnabled());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/pairsPage");
    s.setValue("timeFrame", m_timeFrame);
    s.setValue("timeFrameIndex", ui->timeFrameComboBox->currentIndex());
    s.setValue("tabWidgetIndex", ui->tabWidget->currentIndex());
    s.setValue("pairsTabWidgetIndex", ui->pairsTabWidget->currentIndex());
    s.beginWriteArray("pairsTabWidgetPages");
    for (int i=0;i<ui->pairsTabWidget->count();++i) {
        s.setArrayIndex(i);
        s.setValue("tabText", ui->pairsTabWidget->tabText(i));
        if (i == 0)
            s.setValue("showButton1Enabled", ui->pair1ShowButton->isEnabled());
        else
            s.setValue("showButton2Enabled", ui->pair2ShowButton->isEnabled());
        Ui::ContractDetailsWidget* c = qobject_cast<ContractDetailsWidget*>(ui->pairsTabWidget->widget(i))->getUi();
        s.setValue("securityTypeComboBoxIndex", c->securityTypeComboBox->currentIndex());
        s.setValue("symbol", c->symbolLineEdit->text());
        s.setValue("exchange", c->exchangeLineEdit->text());
        if (c->expiryLineEdit->isEnabled())
            s.setValue("expiry", c->expiryLineEdit->text());
    }
    s.endArray();
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/configPage");
    s.setValue("maPeriod", ui->maPeriodSpinBox->value());
    s.setValue("rsiPeriod", ui->rsiPeriodSpinBox->value());
    s.setValue("stdDevPeriod", ui->stdDevPeriodSpinBox->value());
    s.setValue("volatilityPeriod", ui->volatilityPeriodSpinBox->value());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/tradeEntry");
    s.setValue("managedAccountsComboBoxText", ui->managedAccountsComboBox->currentText());
    s.setValue("amount", ui->tradeEntryAmountSpinBox->value());
    s.setValue("rsiCheckState", ui->tradeEntryRSICheckBox->checkState());
    s.setValue("rsi", ui->tradeEntryRSIUpperSpinBox->value());
    s.setValue("percentFromMeanCheckState", ui->tradeEntryPercentFromMeanCheckBox->checkState());
    s.setValue("percentFromMean", ui->tradeEntryPercentFromMeanSpinBox->value());
    s.setValue("numStdDevLayers", ui->tradeEntryNumStdDevLayersSpinBox->value());
    s.setValue("waitCheckBoxEnabled", ui->waitCheckBox->isEnabled());
    s.setValue("waitCheckBoxState", ui->waitCheckBox->checkState());
    s.setValue("bufferCheckBoxEnabled", ui->layerBufferCheckBox->isEnabled());
    s.setValue("bufferCheckBoxState", ui->layerBufferCheckBox->checkState());
    s.setValue("bufferSpinBoxEnabled", ui->layerBufferDoubleSpinBox->isEnabled());
    s.setValue("buffer", ui->layerBufferDoubleSpinBox->value());
    s.beginWriteArray("layers");
    for (int i=0;i<ui->tradeEntryNumStdDevLayersSpinBox->value();++i) {
        StdDevLayerTab* t = qobject_cast<StdDevLayerTab*>(ui->layersTabWidget->widget(i));
        s.setValue("stdDev", t->getUi()->layerStdDevDoubleSpinBox->value());
        s.setValue("trailCheckBoxEnabled", t->getUi()->layerTrailCheckBox->isEnabled());
        s.setValue("trailCheckBoxCheckState", t->getUi()->layerTrailCheckBox->checkState());
        s.setValue("trail", t->getUi()->layerTrailDoubleSpinBox->value());
        s.setValue("stdDevMinCheckBoxEnabled", t->getUi()->layerStdMinCheckBox->isEnabled());
        s.setValue("stdDevMinCheckBoxState", t->getUi()->layerStdMinCheckBox->checkState());
        s.setValue("stdDevMin", t->getUi()->layerStdMinDoubleSpinBox->value());
    }
    s.endArray();
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/tradeExit");
    s.setValue("percentStopLossCheckBoxState", ui->tradeExitPercentStopLossCheckBox->checkState());
    s.setValue("percentStopLoss", ui->tradeExitPercentStopLossSpinBox->value());
    s.setValue("percentFromMeanCheckState", ui->tradeExitPercentFromMeanCheckBox->checkState());
    s.setValue("percentFromMean", ui->tradeExitPercentFromMeanSpinBox->value());
    s.setValue("stdDevCheckBox", ui->tradeExitStdDevCheckBox->checkState());
    s.setValue("stdDev", ui->tradeExitStdDevDoubleSpinBox->value());
    s.endGroup();
}

void PairTabPage::readSettings()
{
    QSettings s;
    s.beginGroup(m_tabSymbol);
    ui->activateButton->setEnabled(s.value("activateButtonEnabled").toBool());
    ui->deactivateButton->setEnabled(s.value("deactivateButtonEnabled").toBool());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/pairsPage");
    m_timeFrame =(TimeFrame) s.value("timeFrame").toInt();
    ui->timeFrameComboBox->setCurrentIndex(s.value("timeFrameIndex").toInt());
    ui->tabWidget->setCurrentIndex(s.value("tabWidgetIndex").toInt());
    ui->pairsTabWidget->setCurrentIndex(s.value("pairsTabWidgetIndex").toInt());
    int size = s.beginReadArray("pairsTabWidgetPages");
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        ui->pairsTabWidget->setTabText(i, s.value("tabText").toString());
        if (i==0)
            ui->pair1ShowButton->setEnabled(s.value("showButton1Enabled").toBool());
        else
            ui->pair2ShowButton->setEnabled(s.value("showButton2Enabled").toBool());
        Ui::ContractDetailsWidget* c = qobject_cast<ContractDetailsWidget*>(ui->pairsTabWidget->widget(i))->getUi();
        c->securityTypeComboBox->setCurrentIndex(s.value("securityTypeComboBoxIndex").toInt());
        c->symbolLineEdit->setText(s.value("symbol").toString());
        c->exchangeLineEdit->setText(s.value("exchange").toString());
        if (c->expiryLineEdit->isEnabled())
            c->expiryLineEdit->setText(s.value("expiry").toString());
    }
    s.endArray();
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/configPage");
    ui->maPeriodSpinBox->setValue(s.value("maPeriod").toInt());
    ui->rsiPeriodSpinBox->setValue(s.value("rsiPeriod").toInt());
    ui->stdDevPeriodSpinBox->setValue(s.value("stdDevPeriod").toInt());
    ui->volatilityPeriodSpinBox->setValue(s.value("volatilityPeriod").toInt());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/tradeEntry");
    QString accountString = s.value("managedAccountsComboBoxText").toString();
    for (int i=0;i<ui->managedAccountsComboBox->count();++i) {
        if (ui->managedAccountsComboBox->currentText() == accountString) {
            ui->managedAccountsComboBox->setCurrentIndex(i);
            break;
        }
    }
    ui->tradeEntryAmountSpinBox->setValue(s.value("amount").toInt());
    ui->tradeEntryRSICheckBox->setCheckState((Qt::CheckState)s.value("rsiCheckState").toInt());
    ui->tradeEntryRSIUpperSpinBox->setValue(s.value("rsi").toInt());
    ui->tradeEntryPercentFromMeanCheckBox->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckState").toInt());
    ui->tradeEntryPercentFromMeanSpinBox->setValue(s.value("percentFromMean").toInt());
    ui->tradeEntryNumStdDevLayersSpinBox->setValue(s.value("numStdDevLayers").toInt());
    ui->waitCheckBox->setEnabled(s.value("waitCheckBoxEnabled").toBool());
    ui->waitCheckBox->setCheckState((Qt::CheckState)s.value("waitCheckBoxState").toInt());
    ui->layerBufferCheckBox->setEnabled(s.value("bufferCheckBoxEnabled").toBool());
    ui->layerBufferCheckBox->setCheckState((Qt::CheckState)s.value("bufferCheckBoxState").toInt());
    ui->layerBufferDoubleSpinBox->setEnabled(s.value("bufferSpinBoxEnabled").toBool());
    ui->layerBufferDoubleSpinBox->setValue(s.value("buffer").toDouble());
    size = s.beginReadArray("layers");
    for (int i = 0;i<size;++i) {
        StdDevLayerTab* l = new StdDevLayerTab(i, this);
        Ui::StdDevLayerTab* ul = l->getUi();
        ui->layersTabWidget->addTab(l, QString::number(i+1));
        connect(l->getUi()->layerTrailCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(onTrailCheckBoxStateChanged(int)));
        ul->layerStdDevDoubleSpinBox->setValue(s.value("stdDev").toDouble());
        ul->layerTrailCheckBox->setEnabled(s.value("trailCheckBoxEnabled").toBool());
        ul->layerTrailCheckBox->setCheckState((Qt::CheckState)s.value("trailCheckBoxState").toInt());
        ul->layerTrailDoubleSpinBox->setValue(s.value("trail").toDouble());
        ul->layerStdMinCheckBox->setEnabled(s.value("stdDevMinCheckBoxEnabled").toBool());
        ul->layerStdMinCheckBox->setCheckState((Qt::CheckState)s.value("stdDevCheckBoxState").toInt());
        ul->layerStdMinDoubleSpinBox->setValue(s.value("stdDevMin").toDouble());
    }
    s.endArray();
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/tradeExit");
    ui->tradeExitPercentStopLossCheckBox->setCheckState((Qt::CheckState)s.value("percentStopLossCheckBoxState").toInt());
    ui->tradeExitPercentStopLossSpinBox->setValue(s.value("percentStopLoss").toInt());
    ui->tradeExitPercentFromMeanCheckBox->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckBoxState").toInt());
    ui->tradeExitPercentFromMeanSpinBox->setValue(s.value("percentFromMean").toInt());
    ui->tradeExitStdDevCheckBox->setCheckState((Qt::CheckState)s.value("stdDevCheckBox").toInt());
    ui->tradeExitStdDevDoubleSpinBox->setValue(s.value("stdDev").toDouble());
    s.endGroup();
}

TimeFrame PairTabPage::getTimeFrame() const
{
    return m_timeFrame;
}

QString PairTabPage::getTabSymbol() const
{
    return m_tabSymbol;
}






void PairTabPage::placeOrder()
{
    qDebug() << "[DEBUG-placeOrder]";


//    Contract contract;
//    Order order;

//    contract.symbol = "IBM";
//    contract.secType = "STK";
//    contract.exchange = "SMART";
//    contract.currency = "USD";

//    order.action = "BUY";
//    order.totalQuantity = 1000;
//    order.orderType = "LMT";
//    order.lmtPrice = 0.01;

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    Contract* c1 = s1->contract();
    Contract* c2 = s2->contract();

    long orderId1 = m_ibClient->getOrderId();
    long orderId2 = m_ibClient->getOrderId();

    Order* o1 = s1->newSecurityOrder(orderId1)->order;
    Order* o2 = s2->newSecurityOrder(orderId2)->order;

    double ratio1, ratio2;

    if (m_ratio.last() < 1) {
        ratio1 = m_ratio.last();
        ratio2 = 1 - m_ratio.last();
    }
    else {
        ratio1 = s2->getHistData(m_timeFrame)->close.last() / s1->getHistData(m_timeFrame)->close.last();
        ratio2 = 1 - ratio1;
    }

    qDebug() << "[DEBUG-placeOrder] ratio1: " << ratio1 << "ratio2:" << ratio2;

    o1->action = "BUY";
    o1->totalQuantity = (long)ui->tradeEntryAmountSpinBox->value() * ratio1 / s1->getHistData(m_timeFrame)->close.last();
    qDebug() << "[DEBUG-placeOrder] o1->totalQuantity:" << o1->totalQuantity;

    o1->orderType = "MKT";
    o1->transmit = true;
    o1->account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    o2->action = "SELL";
    o2->totalQuantity = (long)ui->tradeEntryAmountSpinBox->value() * ratio2 / s2->getHistData(m_timeFrame)->close.last();
    qDebug() << "[DEBUG-placeOrder] o2->totalQuantity:" << o2->totalQuantity;
    o2->orderType = "MKT";
    o2->transmit = true;
    o2->account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    m_ibClient->placeOrder(orderId1, *c1, *o1);
    m_ibClient->placeOrder(orderId2, *c2, *o2);


//    ComboLeg c1;
//    ComboLeg c2;

//    Security* s1 = m_securityMap.values().at(0);
//    Security* s2 = m_securityMap.values().at(1);

//    c1.conId = s1->contract()->conId;
//    c1.ratio = 1;
//    c1.action = "SELL";
//    c1.exchange = "SMART";
//    c1.openClose = 0;
//    c1.shortSaleSlot = 0;
//    c1.designatedLocation = "";

//    c2.conId = s2->contract()->conId;
//    c2.ratio = 1;
//    c2.action = "BUY";
//    c2.exchange = "SMART";
//    c2.openClose = 0;
//    c2.shortSaleSlot = 0;
//    c2.designatedLocation = "";

//    QList<ComboLeg*> clist;
//    clist.append(&c1);
//    clist.append(&c2);

//    Contract ct1 = *(s1->contract());

//    ct1.symbol = "USD";
//    ct1.secType = "BAG";
//    ct1.comboLegs = clist;

//    Order order;
//    order.action = "BUY";
//    order.totalQuantity = 100;
//    order.orderType = "MKT";

//    long id = m_ibClient->getOrderId();
//    m_ibClient->placeOrder(id, ct1, order);

//    m_ibClient->placeOrder( id, contract, order);

}

void PairTabPage::exitOrder()
{

}

void PairTabPage::showPlot(long tickerId, ChartType chartType)
{
    qDebug() << "[DEBUG-showPlot]";
    Q_UNUSED(chartType);

    Security* s = m_securityMap[tickerId];
    DataVecsHist* dvh = s->getHistData(m_timeFrame);

//    qDebug() << "[DEBUG-showPlot] last dt:" << QDateTime::fromTime_t(dvh->timeStamp.last()).toString("yy.MM.dd/hh:mm:ss")
//             << "numbars:" << dvh->timeStamp.size();

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

//    g->setScatterStyle(QCPScatterStyle::ssCross);

    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    cp->axisRect()->setRangeDrag(Qt::Horizontal);
    cp->axisRect()->setRangeZoom(Qt::Horizontal);

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

    cp->xAxis->setRange(dvh->timeStamp.first(), dvh->timeStamp.last());

    double min = getMin(dvh->close);
    double max = getMax(dvh->close);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->replot();

    qDebug() << "[DEBUG-showPlot] leaving";
}

void PairTabPage::appendPlotsAndTable(long tickerId)
{
    qDebug() << "[DEBUG-appendPlot]";

    int idx = m_securityMap.keys().indexOf(tickerId);
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

    QCustomPlot* cp = m_customPlotMap[idx];
    cp->graph()->addData(dvh->timeStamp.last(), dvh->close.last());
    cp->replot();

    if (m_bothPairsUpdated) {
        m_bothPairsUpdated = false;
        return;
    }

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    DataVecsHist* dvh1 = s1->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = s2->getHistData(m_timeFrame);

    m_ratio = getRatio(dvh1->close, dvh2->close);
    m_ratioMA = getMA(m_ratio, ui->maPeriodSpinBox->value());
    m_ratioStdDev = getMovingStdDev(m_ratio, ui->stdDevPeriodSpinBox->value());
    m_ratioPercentFromMean = getPercentFromMean(m_ratio);
    m_correlation = getCorrelation(dvh1->close, dvh2->close);
    m_ratioVolatility = getVolatility(m_ratio, ui->volatilityPeriodSpinBox->value());
    m_ratioRSI = getRSI(m_ratio, ui->rsiPeriodSpinBox->value());

    QTableWidget* tw = mwui->homeTableWidget;
    QTabWidget* tabWidget = ui->tabWidget;

//    << "Ratio"
//    << "RatioMA"
//    << "StdDev"
//    << "PcntFromMA"
//    << "Corr"
//    << "Vola"
//    << "RSI"

    double ts = dvh->timeStamp.last();

    for (int i=0;i<ui->tabWidget->tabBar()->count();++i) {
        QString tabText = tabWidget->tabText(i);
        cp = qobject_cast<QCustomPlot*>(tabWidget->widget(i));

        if (tabText == "Ratio") {
            cp->graph(0)->addData(ts, m_ratio.last());
            cp->replot();
        }
        else if (tabText == "RatioMA") {
            cp->graph(0)->addData(ts, m_ratioMA.last());
            cp->replot();
        }
        else if (tabText == "StdDev") {
            cp->graph(0)->addData(ts, m_ratioStdDev.last());
            cp->replot();
        }
        else if (tabText == "PcntFrmMean") {
            cp->graph(0)->addData(ts, m_ratioPercentFromMean.last());
            cp->replot();
        }
        else if (tabText == "Corr") {
            cp->graph(0)->addData(ts, m_correlation.last());
            cp->replot();
        }
        else if (tabText == "Vola") {
            cp->graph(0)->addData(ts, m_ratioVolatility.last());
            cp->replot();
        }
        else if (tabText == "RSI") {
            cp->graph(0)->addData(ts, m_ratioRSI.last());
            cp->replot();
        }
    }

    QString sym1 = ui->pairsTabWidget->tabText(0);
    QString sym2 = ui->pairsTabWidget->tabText(1);

    int row = 0;

//    << "Ratio"
//    << "RatioMA"
//    << "StdDev"
//    << "PcntFromMA"
//    << "Corr"
//    << "Vola"
//    << "RSI";

    for (int r=0;r<tw->rowCount();++r) {
        QTableWidgetItem* item1 = tw->item(r,0);
        QTableWidgetItem* item2 = tw->item(r,1);
        if (item1->text() == sym1 && item2->text() == sym2)
            row = r;
    }

    for (int c=0;c<tw->columnCount();++c) {
        QTableWidgetItem* headerItem = tw->horizontalHeaderItem(c);
        if (headerItem->text() == "Ratio")
            tw->item(row,c)->setText(QString::number(m_ratio.last()));
        else if (headerItem->text() == "RatioMA")
            tw->item(row,c)->setText(QString::number(m_ratioMA.last()));
        else if (headerItem->text() == "StdDev")
            tw->item(row,c)->setText(QString::number(m_ratioStdDev.last()));
        else if (headerItem->text() == "PcntFromMA")
            tw->item(row,c)->setText(QString::number(m_ratioPercentFromMean.last()));
        else if (headerItem->text() == "Corr")
            tw->item(row,c)->setText(QString::number(m_correlation.last()));
        else if (headerItem->text() == "Vola")
            tw->item(row,c)->setText(QString::number(m_ratioVolatility.last()));
        else if (headerItem->text() == "RSI")
            tw->item(row,c)->setText(QString::number(m_ratioRSI.last()));
    }

    m_bothPairsUpdated = true;
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
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "RatioMA");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "StdDev");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "PcntFrmMA");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "Corr");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "Vola");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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

    int tabIdx = ui->tabWidget->addTab(cp, "RSI");
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

    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        cp->xAxis->setDateTimeFormat("MM/dd/yy");
    else
        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

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
    qDebug() << "[DEBUG-addTableRow]";

    QTableWidget* tab = mwui->homeTableWidget;

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    DataVecsHist* d1 = s1->getHistData(m_timeFrame);
    DataVecsHist* d2 = s2->getHistData(m_timeFrame);

//    m_headerLabels << "Pair1"
//            << "Pair2"
//            << "Price1"
//            << "Price2"
//            << "Ratio"
//            << "RatioMA"
//            << "StdDev"
//            << "PcntFromMA"
//            << "Corr"
//            << "Vola"
//            << "RSI";





    QStringList itemList;
    itemList << m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text()
             << m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text()
             << m_timeFrameString
             << QString::number(d1->close.last())
             << QString::number(d2->close.last())
             << QString::number(m_ratio.last())
             << QString::number(m_ratioMA.last())
             << QString::number(m_ratioStdDev.last())
             << QString::number(m_ratioPercentFromMean.last())
             << QString::number(m_correlation.last())
             << QString::number(m_ratioVolatility.last())
             << QString::number(m_ratioRSI.last())
                ;

//    qDebug() << "itemList:" << itemList;
    int numRows = tab->rowCount();

    qDebug() << "[DEBUG-addTableRow] numRows:" << numRows;

    if (numRows == 0)
        m_homeTablePageRowIndex = 0;
    else
        m_homeTablePageRowIndex = numRows;
    tab->setRowCount(m_homeTablePageRowIndex + 1);
    for (int i=0;i<itemList.size();++i) {
        QTableWidgetItem* item = new QTableWidgetItem(itemList.at(i));
        tab->setItem(m_homeTablePageRowIndex, i, item);
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











































