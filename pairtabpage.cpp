#include "pairtabpage.h"
#include "ui_pairtabpage.h"
#include "ui_mainwindow.h"
#include "ui_stddevlayertab.h"
#include "ui_contractdetailswidget.h"
#include "ui_globalconfigdialog.h"
#include "ui_datatoolboxwidget.h"

#include "contractdetailswidget.h"
#include "globalconfigdialog.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "qcustomplot.h"
#include "helpers.h"
#include "security.h"
#include "mainwindow.h"
#include "stddevlayertab.h"
//#include "smtp.h"
#include "datatoolboxwidget.h"
#include "tablewidgetitem.h"

#include <QDateTime>
#include <QTime>
#include <QSpinBox>
#include <QPair>
#include <QPen>
#include <QColor>
#include <QTableWidget>
//#include <QTableWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTimeZone>
#include <QCoreApplication>
#include <QCursor>

int PairTabPage::PairTabPageCount = 0;
QMultiMap<long, Security*> PairTabPage::RawDataMap = QMultiMap<long, Security*>();
bool PairTabPage::DontClickShowButtons = false;

PairTabPage::PairTabPage(IBClient *ibClient, const QStringList & managedAccounts, QWidget *parent)
    : QWidget(parent)
    , m_ibClient(ibClient)
    , m_managedAccounts(managedAccounts)
    , ui(new Ui::PairTabPage)
    , m_ratioRSITriggerActivated(false)
    , m_percentFromMeanTriggerActivated(false)
    , m_homeTablePageRowIndex(-1)
    , m_gettingMoreHistoricalData(false)
    , m_bothPairsUpdated(true)
    , m_tabSymbol(QString())
    , m_canSetTabWidgetCurrentIndex(false)
    , m_readingSettings(false)
    , m_pair1ShowButtonClickedAlready(false)
    , m_pair2ShowButtonClickedAlready(false)
    , m_pairTabPageId(++PairTabPageCount)
{
//    qDebug() << "[DEBUG-PairTabPage]";

    ui->setupUi(this);

    m_mainWindow = qobject_cast<MainWindow*>(parent);
    mwui = m_mainWindow->getUi();

    m_origButtonStyleSheet = ui->activateButton->styleSheet();

    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);

    ui->pair1UnitOverrideLabel->setVisible(false);
    ui->pair1UnitOverrideSpinBox->setVisible(false);
    ui->pair2UnitOverrideLabel->setVisible(false);
    ui->pair2UnitOverrideSpinBox->setVisible(false);

    ui->lookbackLabel->setVisible(false);
    ui->lookbackSpinBox->setVisible(false);


    ui->pair1UnitOverrideLabel->setText(ui->pairsTabWidget->tabText(0) + QString(" Units"));
    ui->pair2UnitOverrideLabel->setText(ui->pairsTabWidget->tabText(1) + QString(" Units"));

    ui->pair2Tab->setVisible(false);

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

//    cdui1->expiryLabel->setEnabled(false);
    cdui1->expiryLabel->setVisible(false);
//    cdui2->expiryLabel->setEnabled(false);
    cdui2->expiryLabel->setVisible(false);


    cdui1->expiryMonthSpinBox->setVisible(false);
    cdui1->expiryYearSpinBox->setVisible(false);

    cdui2->expiryMonthSpinBox->setVisible(false);
    cdui2->expiryYearSpinBox->setVisible(false);

    cdui1->securityTypeComboBox->addItems(secTypes);
    cdui2->securityTypeComboBox->addItems(secTypes);


    setDefaults();


    cdui1->symbolLineEdit->setText("MSFT");
//    cdui1->primaryExchangeLineEdit->setText("NYSE");

    cdui2->symbolLineEdit->setText("AAPL");
//    cdui2->primaryExchangeLineEdit->setText("NASDAQ");

    ui->pairsTabWidget->tabBar()->setTabText(0, cdui1->symbolLineEdit->text());
    ui->pairsTabWidget->tabBar()->setTabText(1, cdui2->symbolLineEdit->text());

//    ui->managedAccountsComboBox->addItems(m_managedAccounts);

//    qDebug() << "[DEBUG-PairTabPage]" << ui->pair1ShowButton->styleSheet();
//    ui->pair1ShowButton->setStyleSheet("background-color:green");
//    ui->pair2ShowButton->setEnabled(false);


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
    connect(ui->overrideUnitSizeCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(onOverrideCheckBoxStateChanged(int)));
//    connect(ui->pair1ResetButton, SIGNAL(pressed()),
//            this, SLOT(onPair1ResetButtonClicked()));
//    connect(ui->pair2ResetButton, SIGNAL(pressed()),
//            this, SLOT(onPair2ResetButtonClicked()));

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

//    if (!(m_securityMap.keys().contains(reqId)
//          || m_newBarMap.keys().contains(reqId)
//          || m_moreDataMap.keys().contains(reqId)))
//        return;

    Security* s= NULL;
    long sid = 0;
//    bool isS1 = false;
    bool isS2 = false;
    double timeStamp = 0;
    bool isNewBarReq = false;
    bool isMoreDataReq = false;

    // hack to remove mysterious m_securityMap entry
    if (m_securityMap.count() > 2)
        for (int i=0;i<m_securityMap.count();++i) {
            if (m_securityMap.keys().at(i) == 0 && m_securityMap.values().at(i) == NULL)
                m_securityMap.remove(i);
        }

    if (m_timeFrame == DAY_1)
        timeStamp = (double)QDateTime::fromString(date, "yyyyMMdd").toTime_t();
    else
        timeStamp = date.toDouble();


    if (m_securityMap.keys().contains(reqId)) {
        s = m_securityMap.value(reqId);
        sid = reqId;
//        qDebug() << "[DEBUG-onHistoricalData] initial data request";
    }
    else if (m_newBarMap.values().contains(reqId)) {
        isNewBarReq = true;
        sid = m_newBarMap.key(reqId);
        s = m_securityMap.value(sid);
//        qDebug() << "[DEBUG-onHistoricalData] new bar data request";
    }
    else if (m_moreDataMap.values().contains(reqId)) {
        isMoreDataReq = true;
        sid = m_moreDataMap.key(reqId);
        s = m_securityMap.value(sid);
//        qDebug() << "[DEBUG-onHistoricalData] more data request";
    }
    else {
//        qDebug() << "[ERROR-onHistoricalData] request Id UNKNOWN";
//        qDebug() << "    secMapKeys:" << m_securityMap.keys();
//        qDebug() << "    secMapVals:" << m_securityMap.values();
//        qDebug() << "    reqId:" << reqId;
        return;
    }

//    if (m_securityMap.keys().indexOf(reqId) == 0) {
//        isS1 = true;
//    }
    if (m_securityMap.keys().indexOf(reqId) == 1) {
        isS2 = true;
    }

    if (!isNewBarReq) {
        if (date.startsWith("finished")) {
            DataVecsHist* dvh = s->getHistData(m_timeFrame);
            if (!isMoreDataReq) {
                double lastBarsTimeStamp = dvh->timeStamp.last();
                s->setLastBarsTimeStamp(lastBarsTimeStamp);

                // realtime data request

                // is this a duplicate?
                long tid = 0;

                QMap<long, Security*> tmpMap = PairTabPage::RawDataMap;

                for (int i=0;i<tmpMap.values().count();++i) {
                    Security* ss = tmpMap.values().at(i);
                    if ((ss->contract()->symbol == s->contract()->symbol)
                            && (ss->contract()->expiry == s->contract()->expiry)) {
                        tid = tmpMap.key(ss);
                        PairTabPage::RawDataMap.insert(tid, s);
                    }
                }

                if (tid == 0) {
                    tid = m_ibClient->getTickerId();
                    PairTabPage::RawDataMap.insert(tid, s);
                    m_ibClient->reqMktData(tid, *(s->contract()), QByteArray(""), false);
                    s->getTimer()->start(m_timeFrameInSeconds * 1000);
                }
                s->setRealTimeTickerId(tid);


qDebug() << "[DEBUG-onHistoricalData] NUM BARS RECEIVED:" << dvh->timeStamp.size()
                         << "Last timestamp:" << QDateTime::fromTime_t( (int)dvh->timeStamp.last()).toString("yyMMdd::hh:mm:ss");

//                if (dvh->timeStamp.size() < ui->lookbackSpinBox->value()) {
////                    // FIXME: I need more bars (1 HOUR BARS)
//                    m_moreDataMap[s->tickerId()] = m_ibClient->getTickerId();
////                    QTimer::singleShot(1000, this, SLOT(onMoreHistoricalDataNeeded()));
//                    onMoreHistoricalDataNeeded();
//                    return;
//                }
            }
            else {  // HANDLE THE LOOKBACK DATA
                m_moreDataMap.remove(s->getHistoricalTickerId());
                if (dvh->timeStamp.size() < ui->lookbackSpinBox->value()) {
                    m_moreDataMap[s->getHistoricalTickerId()] = m_ibClient->getTickerId();
                    onMoreHistoricalDataNeeded();
                    return;
                }
            }

//    qDebug() << "[DEBUG-" << __func__ << "]" << s->contract()->symbol << "lastBarsTimeStamp:" << (uint)s->getLastBarsTimeStamp();

            showPlot(sid);

            if (isS2 && m_securityMap.values().at(0)->getHistData(m_timeFrame)) {

                // handle different sized data sets.. TEMPORARY... FIX ME!!!
                int s1 = m_securityMap.values().at(0)->getHistData(m_timeFrame)->timeStamp.size();
                int s2 = m_securityMap.values().at(1)->getHistData(m_timeFrame)->timeStamp.size();

                if (s1 > s2) {
                    m_securityMap.values().at(0)->fixHistDataSize(m_timeFrame, s1-s2);
                }
                else
                    m_securityMap.values().at(1)->fixHistDataSize(m_timeFrame, s2-s1);


                plotRatio();

//                plotRatioMA();

                plotRatioStdDev();

                plotRatioPercentFromMean();

                plotCorrelation();

//                plotCointegration();

                plotRatioVolatility();

                plotRatioRSI();

                plotRSISpread();

                removeTableRow();
                addTableRow();

                pDebug("done with plots");

                if (m_canSetTabWidgetCurrentIndex) {
                    QSettings s;
                    s.beginGroup(m_tabSymbol);
//                    ui->tabWidget->setCurrentIndex(s.value("tabWidgetIndex").toInt());
                    s.endGroup();
                    m_canSetTabWidgetCurrentIndex = false;
                }
                ui->mdiArea->tileSubWindows();
                ui->mdiArea->setSubWindowHeight(ui->mdiArea->subWindowList().first()->height());
                ui->mdiArea->setSubWindowWidth( ui->mdiArea->subWindowList().first()->width());
            }
            else {
                ui->mdiArea->subWindowList().at(0)->showMaximized();
            }
        }
        else {
            if (isMoreDataReq) {
//                qDebug() << "[DEBUG-onHistoricalData]" << m_timeFrame << QDateTime::fromTime_t((int)timeStamp).toString("yyyyMMdd/hh:mm:ss") << open << high << low << close << volume << barCount << WAP << hasGaps;
                s->appendMoreBarData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);

            }
            else {
//                qDebug() << "[DEBUG-onHistoricalData]" << m_timeFrame << QDateTime::fromTime_t((int)timeStamp).toString("yyyyMMdd/hh:mm:ss") << open << high << low << close << volume << barCount << WAP << hasGaps;
                s->appendHistData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
            }
        }
    }
    else {
        if (date.startsWith("finished")) {
            pDebug("isNewDataRequest");
            s->handleNewBarData(m_timeFrame);
            if (ui->activateButton->isEnabled()) {
                checkTradeTriggers();
                if (!s->getSecurityOrderMap()->isEmpty()) {
                    checkTradeExits();
                }
            }
            appendPlotsAndTable(sid);

            uint diffSeconds = 0;
            uint nowTimeStamp = QDateTime::currentDateTime().toTime_t();

            for (;nowTimeStamp % m_timeFrameInSeconds != 0;--nowTimeStamp) {
                diffSeconds++;
            }

            if (diffSeconds) {
                s->getTimer()->start((m_timeFrameInSeconds - diffSeconds) * 1000);
            }
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
    bool pair2ShowButtonClickedAlready = m_pair2ShowButtonClickedAlready;

    if (m_pair1ShowButtonClickedAlready) {
        reqDeletePlotsAndTableRow();
    }
    ui->pair2Tab->setVisible(true);
//    ui->pair1ResetButton->setVisible(true);
    ui->pair2ShowButton->setEnabled(true);

    if (m_securityMap.keys().size() >= 2) {
//qDebug() << "[DEBUG-on_pair1ShowButton_clicked()] secKeys:" << m_securityMap.keys();
        return;
    }
    long tickerId = m_ibClient->getTickerId();
    Security* s = new Security(tickerId, this);
    m_securityMap[tickerId] = s;

//qDebug() << "[DEBUG-on_pair1ShowButtonClicked] m_securityMap.size():" << m_securityMap.size();

    connect(s->getTimer(), SIGNAL(timeout()),
            this, SLOT(onPair1TimeOut()));

    Contract* c = s->contract();
//    c->conId = 0;
    c->secType = m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText().toLocal8Bit();
    c->exchange = m_pair1ContractDetailsWidget->getUi()->exchangeLineEdit->text().toLocal8Bit();
//    c->primaryExchange = m_pair1ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    c->currency = m_pair1ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();
    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == "STK") {
        c->symbol = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
    }
    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText()=="FUT") {
        c->symbol = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text().toLocal8Bit();
        c->expiry = QString(QString::number(m_pair1ContractDetailsWidget->getUi()->expiryYearSpinBox->value())
                    + QString::number(m_pair1ContractDetailsWidget->getUi()->expiryMonthSpinBox->value())).toLocal8Bit();
//        c->localSymbol = m_pair1ContractDetailsWidget->getUi()->localSymbolLineEdit->text().toLocal8Bit();
    }

    long reqId = m_ibClient->getTickerId();
    m_contractDetailsMap[tickerId] = reqId;
    m_ibClient->reqContractDetails(reqId, *c);

//    ui->pair2ShowButton->setEnabled(true);
//    ui->pair1ShowButton->setEnabled(false);

//    ui->timeFrameComboBox->setEnabled(false);

    if (pair2ShowButtonClickedAlready) {
        ui->pair2ShowButton->click();
    }

    m_pair1ShowButtonClickedAlready = true;
    ui->chartDataPage->getUi()->lastPair1PriceLabel->setText(c->symbol);

//qDebug() << "[DEBUG-on_pair1ShowButton_clicked] leaving";
}


void PairTabPage::on_pair2ShowButton_clicked()
{
    pDebug("");

//    ui->pair2ShowButton->setEnabled(false);
//    ui->pair2ResetButton->setVisible(true);

    if (m_pair2ShowButtonClickedAlready) {
        reqDeletePlotsAndTableRow();
        ui->pair1ShowButton->click();
    }


    if (m_securityMap.keys().size() >= 2)
        return;
    long tickerId = m_ibClient->getTickerId();
    Security* pair2 = new Security(tickerId, this);
    m_securityMap[tickerId] = pair2;

    m_securityMap.values().at(0)->setPairPartner(m_securityMap.values().at(1));
    m_securityMap.values().at(1)->setPairPartner(m_securityMap.values().at(0));

//qDebug() << "[DEBUG-on_pair2ShowButtonClicked] m_securityMap.size():" << m_securityMap.size();

    setTabSymbol();
    mwui->tabWidget->setTabText(mwui->tabWidget->indexOf(this),
                                m_tabSymbol);
    mwui->tabWidget->setCurrentIndex(mwui->tabWidget->count()-1);

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
//    contract->primaryExchange = m_pair2ContractDetailsWidget->getUi()->primaryExchangeLineEdit->text().toLocal8Bit();
    contract->currency = m_pair2ContractDetailsWidget->getUi()->currencyLineEdit->text().toLocal8Bit();
    contract->expiry = QByteArray::number( m_pair2ContractDetailsWidget->getUi()->expiryYearSpinBox->value())
            + QByteArray::number(m_pair2ContractDetailsWidget->getUi()->expiryMonthSpinBox->value());
//    contract->localSymbol = m_pair1ContractDetailsWidget->getUi()->localSymbolLineEdit->text().toLocal8Bit();
//    contract->strike = m_pair1ContractDetailsWidget->getUi()->strikeLineEdit->text().toDouble();
//    contract->right = m_pair1ContractDetailsWidget->getUi()->rightLineEdit->text().toLocal8Bit();
//    contract->multiplier = m_pair1ContractDetailsWidget->getUi()->multiplierLineEdit->text().toLocal8Bit();

    long reqId = m_ibClient->getTickerId();
    m_contractDetailsMap[tickerId] = reqId;
    m_ibClient->reqContractDetails(reqId, *contract);

    m_pair2ShowButtonClickedAlready = true;

    ui->chartDataPage->getUi()->lastPair2PriceLabel->setText(contract->symbol);

    pDebug("leaving");
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
//    P_DEBUG;

    long sid = m_securityMap.keys().first();
    Security* s = m_securityMap.value(sid);

    if (!isTrading(s))
        return;

    static bool isFirst = true;
    long tid = -1;    

    if (isFirst) {
        tid = m_ibClient->getTickerId();
        m_newBarMap[sid] = tid;
        reqHistoricalData(tid);
        isFirst = false;
    }
    else {
        s->handleRawBarData();
//        appendPlotsAndTable(sid);
    }
}

void PairTabPage::onPair2TimeOut()
{
//    pDebug("");

    long sid = m_securityMap.keys().last();
    Security* s = m_securityMap.value(sid);

    if (!isTrading(s))
        return;

    static bool isFirst = true;
    long tid = -1;

    if (isFirst) {
        tid = m_ibClient->getTickerId();
        m_newBarMap[sid] = tid;
        reqHistoricalData(m_newBarMap[sid]);
        isFirst = false;
    }
    else {
        s->handleRawBarData();
//        appendPlotsAndTable(sid);
    }
}

void PairTabPage::onActivateButtonClicked(bool)
{
    if (ui->manualTradeEntryCheckBox->isChecked()) {
        QMessageBox msgBox;
        msgBox.setText("Caution!");
        msgBox.setInformativeText("You are about to place an order... are you sure about the configurations in the trade entry window that pertain to manual orders?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;
        placeOrder(MANUAL);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Caution!");
        msgBox.setInformativeText("Are you sure all trade entries and exits are ok?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;
        for (int i=0;i<5;++i) {
            m_stdDevLayerPeaks.append(m_ratioStdDev.last());
        }
        checkTradeTriggers();
    }

    ui->activateButton->setEnabled(false);
    ui->deactivateButton->setEnabled(true);
//    ui->deactivateButton->setStyleSheet("background-color:red");
//    ui->activateButton->setAutoFillBackground(true);
//    ui->activateButton->setPalette(ui->activateButton->parentWidget()->palette());




    // THIS IS ONLY FOR TESTING REMOVE
//    placeOrder(TEST);

//    QTimer::singleShot(10000, this, SLOT(onSingleShotTimer()));

}

void PairTabPage::onDeactivateButtonClicked(bool)
{
//    m_ibClient->reqOpenOrders();
    if (m_securityMap.count() > 2) {
        for (int i=0;i<m_securityMap.count();++i) {
            if (m_securityMap.keys().at(i) == 0 && m_securityMap.values().at(i) == NULL)
                m_securityMap.remove(m_securityMap.keys().at(i));
        }
    }
    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    if (!ui->manualTradeExitCheckBox->isChecked()) {
        if (s1->getSecurityOrderMap()->isEmpty() && s2->getSecurityOrderMap()->isEmpty()) {
            ui->activateButton->setEnabled(true);
            ui->deactivateButton->setEnabled(false);
            m_stdDevLayerPeaks.clear();
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Can not deactivate this pair because orders have been placed");
            msgBox.setInformativeText("Please go to the orders page to close existing orders");
            /*int ret =*/ msgBox.exec();
        }
    }
    else {
        if (s1->getSecurityOrderMap()->isEmpty() && s2->getSecurityOrderMap()->isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText("Can not close order, because no order is currently recorded");
            msgBox.setInformativeText("If this is a faulty condition, see app creator for fix");
            /*int ret =*/ msgBox.exec();
        }
        else {
            ui->activateButton->setEnabled(true);
            ui->deactivateButton->setEnabled(false);
            exitOrder();
        }
    }
}



void PairTabPage::onSingleShotTimer()
{
//qDebug() << "[DEBUG-onSingleShotTimer]";
//    m_ibClient->reqOpenOrders();
}

void PairTabPage::onContractDetails(int reqId, const ContractDetails &contractDetails)
{
//qDebug() << "[DEBUG-onContractDetails]"
//             << reqId
//             << contractDetails.summary.symbol
//             << contractDetails.summary.localSymbol
//             << contractDetails.summary.conId
//             << contractDetails.summary.expiry;
//             << "exchange:" << contractDetails.summary.exchange
//             << "primaryExchange:" << contractDetails.summary.primaryExchange
//             << "liquidHours:" << contractDetails.liquidHours
//             << "tradingHours:" << contractDetails.tradingHours
//             << "timeZoneId" << contractDetails.timeZoneId;
//            ;
//    qApp->exit();


//    QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();

//    foreach(QByteArray zone, zones)
//        pDebug(zone);

//    qApp->exit();

    Security* s = m_securityMap.value(m_contractDetailsMap.key(reqId));

    if (!s) {
//qDebug() << "[WARN-onContractDetails]"
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
    setTabSymbol();
    mwui->tabWidget->setTabText(mwui->tabWidget->indexOf(this),
                                m_tabSymbol);
    ui->pair1UnitOverrideLabel->setText(text.toUpper() + QString(" Units"));
}

void PairTabPage::onCdui2SymbolTextChanged(QString text)
{
    ui->pairsTabWidget->setTabText(1, text.toUpper());
//    m_tabSymbol = ui->pairsTabWidget->tabText(0) + "/" + text.toUpper() + "(" + m_timeFrameString + ")";
    setTabSymbol();
    mwui->tabWidget->setTabText(mwui->tabWidget->indexOf(this),
                                m_tabSymbol);
    ui->pair2UnitOverrideLabel->setText(text.toUpper() + QString(" Units"));

}



ContractDetailsWidget *PairTabPage::getPair2ContractDetailsWidget() const
{
    return m_pair2ContractDetailsWidget;
}

bool PairTabPage::reqClosePair()
{
    // pDebug("");
//qDebug() << "[DEBUG-reqClosePair]";

    Security* s1=NULL;
    Security* s2=NULL;

    if (m_securityMap.size() == 0)
        return true;

    // THIS IS A HACK FOR AN BAD VALUE AT MAP INDEX 0... FIXME !!!;
    if (m_securityMap.size() == 1)
        s1 = m_securityMap.values().at(0);
    if (m_securityMap.keys().size() == 2) {
        s1 = m_securityMap.values().at(0);
        s2 = m_securityMap.values().at(1);
    }
    else if (m_securityMap.keys().size() == 3) {
        pDebug("I SHOULD NOT BE HERE... FIXME FIXME FIXME !!!!");
        s1 = m_securityMap.values().at(1);
        s2 = m_securityMap.values().at(2);
    }

    QSettings s;

    int numOfSameSecurity = 0;

    // IF S2 NOT SET YET
    if (s1 != NULL && s2 == NULL) {
        s.remove(m_tabSymbol);
        if (s1->getTimer()->isActive()) {
            s1->getTimer()->stop();
        }

        for (int i=0;i<PairTabPage::RawDataMap.values().count();++i) {
            Security* ss = PairTabPage::RawDataMap.values().at(i);
//            if (ss->contract()->symbol == s1->contract()->symbol
//                    && ss->contract()->expiry == s1->contract()->expiry)
//            {
//                long key = PairTabPage::RawDataMap.key(ss);
//                if (PairTabPage::RawDataMap.contains(key, ss))
//                    PairTabPage::RawDataMap.remove(key, ss);
//                ++numOfSameSecurity;
//            }
            if (ss == s1) {
                long key = PairTabPage::RawDataMap.key(ss);
                if (PairTabPage::RawDataMap.contains(key, ss))
                    PairTabPage::RawDataMap.remove(key, ss);
                ++numOfSameSecurity;
            }
        }

        if (numOfSameSecurity == 1) {
            m_ibClient->cancelMktData(s1->getRealTimeTickerId());
        }
        return true;
    }
    if (s1->getSecurityOrderMap()->isEmpty()
            && s2->getSecurityOrderMap()->isEmpty()) {
        if (mwui->homeTableWidget->rowCount() > 0
                && m_homeTablePageRowIndex != -1) {
            QTableWidget* tw = mwui->homeTableWidget;
            for (int r=0;r<tw->rowCount();++r) {
                if (tw->item(r,0)->text() == m_tabSymbol) {
                    tw->removeRow(r);
                    tw->update();
                }
            }
        }
        if (s.contains(m_tabSymbol))
            s.remove(m_tabSymbol);
        if (s1->getTimer()->isActive()) {
            s1->getTimer()->stop();
        }
        for (int i=0;i<PairTabPage::RawDataMap.values().count();++i) {
            Security* ss = PairTabPage::RawDataMap.values().at(i);
//            if (ss->contract()->symbol == s1->contract()->symbol) {
//                if (ss->contract()->secType == QByteArray("FUT")
//                        && s1->contract()->secType == QByteArray("FUT")) {
//                    if (ss->contract()->expiry == s1->contract()->expiry) {
//                        long key = PairTabPage::RawDataMap.key(ss);
//                        if (PairTabPage::RawDataMap.contains(key, ss))
//                            PairTabPage::RawDataMap.remove(key, ss);
//                        ++numOfSameSecurity;
//                    }
//                }
//                else {
//                    long key = PairTabPage::RawDataMap.key(ss);
//                    if (PairTabPage::RawDataMap.contains(key, ss))
//                        PairTabPage::RawDataMap.remove(key, ss);
//                    ++numOfSameSecurity;
//                }
//            }
            if (ss == s1) {
                long key = PairTabPage::RawDataMap.key(ss);
                if (PairTabPage::RawDataMap.contains(key, ss))
                    PairTabPage::RawDataMap.remove(key, ss);
                ++numOfSameSecurity;
            }
        }
        if (numOfSameSecurity == 1) {
            m_ibClient->cancelMktData(s1->getRealTimeTickerId());
        }
        if (s2->getTimer()->isActive()) {
            s2->getTimer()->stop();
        }
        numOfSameSecurity = 0;
        for (int i=0;i<PairTabPage::RawDataMap.values().count();++i) {
            Security* ss = PairTabPage::RawDataMap.values().at(i);
            if (ss == s2)
            {
                long key = PairTabPage::RawDataMap.key(ss);
                if (PairTabPage::RawDataMap.contains(key, ss))
                    PairTabPage::RawDataMap.remove(key, ss);
                ++numOfSameSecurity;
            }
        }
        if (numOfSameSecurity == 1) {
            m_ibClient->cancelMktData(s2->getRealTimeTickerId());
        }
        m_securityMap.remove(m_securityMap.key(s1));
        m_securityMap.remove(m_securityMap.key(s2));
        return true;
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Notice!");
        msgBox.setInformativeText("This pair page has open orders and will not be closed until the orders are closed");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return false;
    }
}

ContractDetailsWidget *PairTabPage::getPair1ContractDetailsWidget() const
{
    return m_pair1ContractDetailsWidget;
}


void PairTabPage::reqHistoricalData(long tickerId, QDateTime dt)
{
//    qDebug() << "[DEBUG-reqHistoricalData]";

    TimeFrame tf = (TimeFrame)ui->timeFrameComboBox->currentIndex();
    QByteArray barSize = ui->timeFrameComboBox->currentText().toLocal8Bit();
    QByteArray durationStr;
    bool isNewBarReq = false;
    Security* security;

    if (m_newBarMap.values().contains(tickerId)) {
        security = m_securityMap.value(m_newBarMap.key(tickerId));
        isNewBarReq = true;
    }
    else if (m_securityMap.keys().contains(tickerId)) {
        security = m_securityMap.value(tickerId);
    }
    else if (m_moreDataMap.values().contains(tickerId)) {
        security = m_securityMap.value(m_moreDataMap.key(tickerId));
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

    /// THIS IS NOT SET... SHOULD IT BE?
//    if (m_timeFrame == DAY_1) {
//        dt = dt.addDays(-1);
//    }
//    else {
//        QTime t(dt.time());
//        t = t.addSecs(-m_timeFrameInSeconds);
//        dt.setTime(t);
//    }
    /// END.. THIS IS NOT SET...

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
//qDebug() << "[DEBUG-onMoreHistoricalDataNeeded]";
    m_gettingMoreHistoricalData = true;
    Security* s = m_securityMap.value(m_moreDataMap.keys().last());
    double firstTimeStamp = s->getHistData(m_timeFrame)->timeStamp.first();
    QDateTime firstDT = QDateTime::fromTime_t((uint)firstTimeStamp);

    // THIS HAS TO BE WRONG .. FIXME !!!!!!!!!
    long id = m_moreDataMap[m_moreDataMap.values().last()];
    reqHistoricalData(id, firstDT);
}

void PairTabPage::onContextMenuRequest(QPoint point)
{
    QMenu menu(this);
    if (ui->mdiArea->viewMode() == QMdiArea::TabbedView) {
        menu.addAction("Cascade View", ui->mdiArea, SLOT(onCascadeAct()));
        menu.addAction("Tiled View", ui->mdiArea, SLOT(onTiledAct()));
    }
    else {
        menu.addAction("Cascade View", ui->mdiArea, SLOT(onCascadeAct()));
        menu.addAction("Tabbed View", ui->mdiArea, SLOT(onTabbedAct()));
        menu.addAction("Tiled View", ui->mdiArea, SLOT(onTiledAct()));
    }
    menu.addAction("Reset Plot", this, SLOT(onResetPlot()));

    menu.exec(mapToGlobal(point));
}

void PairTabPage::onCascadeAct()
{
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
    ui->mdiArea->cascadeSubWindows();
    m_mdiCascade = true;
    m_mdiTile = false;
    QMdiSubWindow* w;
    foreach(w, ui->mdiArea->subWindowList())
        w->resize(ui->mdiArea->subWindowWidth(), ui->mdiArea->subWindowHeight());
}

void PairTabPage::onTiledAct()
{
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
    ui->mdiArea->tileSubWindows();
}

void PairTabPage::onTabbedAct()
{
    ui->mdiArea->setViewMode(QMdiArea::TabbedView);
}

void PairTabPage::onOverrideCheckBoxStateChanged(int checkState)
{
    bool visible;
    if (checkState == Qt::Checked)
        visible = true;
    else
        visible = false;
    ui->pair1UnitOverrideLabel->setVisible(visible);
    ui->pair1UnitOverrideSpinBox->setVisible(visible);
    ui->pair2UnitOverrideLabel->setVisible(visible);
    ui->pair2UnitOverrideSpinBox->setVisible(visible);
}

void PairTabPage::onMailSent(const QString &msg)
{
    Q_UNUSED(msg);
    //qDebug() << "[DEBUG-onMailSent]" << msg;
}


uint PairTabPage::getTimeFrameInSeconds() const
{
    return m_timeFrameInSeconds;
}



void PairTabPage::setTabSymbol(const QString &tabSymbol)
{
    m_tabSymbol = tabSymbol;
}

void PairTabPage::setTabSymbol()
{
    QString exp1, exp2;

    QString idString(QString::number(m_pairTabPageId));

    QString expiryString1 = QString::number(m_pair1ContractDetailsWidget->getUi()->expiryMonthSpinBox->value())
            + "/"
            + QString::number(m_pair1ContractDetailsWidget->getUi()->expiryYearSpinBox->value());

    QString expiryString2 = QString::number(m_pair2ContractDetailsWidget->getUi()->expiryMonthSpinBox->value())
            + "/"
            + QString::number(m_pair2ContractDetailsWidget->getUi()->expiryYearSpinBox->value());

    if (ui->pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        exp1 = " [" + expiryString1 + "]";
    }
    if (ui->pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        exp2 = " [" + expiryString2 + "]";
    }
    m_oldTabSymbol = m_tabSymbol;
    m_tabSymbol = idString
            + ": "
            + ui->pairsTabWidget->tabText(0)
            + exp1
            + " / "
            + ui->pairsTabWidget->tabText(1)
            + exp2
            + " (" + m_timeFrameString + ")";
}

QList<Security *> PairTabPage::getSecurities()
{
    QList<Security*> l;
    if (m_securityMap.count() > 2 && m_securityMap.keys().at(0) == 0 && m_securityMap.values().at(0) == NULL) {
        m_securityMap.remove(0);
    }
    return m_securityMap.values();
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
//qDebug() << "[DEBUG-PairTabPage::writeSettings]";

    QSettings s;
    s.beginGroup(m_tabSymbol);
    s.setValue("activateButtonEnabled", ui->activateButton->isEnabled());
    s.setValue("dectivateButtonEnabled", ui->deactivateButton->isEnabled());
    s.setValue("manualTradeEntryCheckBoxState", ui->manualTradeEntryCheckBox->checkState());
    s.setValue("manualTradeExitCheckBoxState", ui->manualTradeExitCheckBox->checkState());
    s.setValue("toolBoxIndex", ui->toolBox->currentIndex());
//    s.setValue("tabWidgetIndex", ui->tabWidget->currentIndex());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/pairsPage");

    s.setValue("timeFrame", m_timeFrame);
    s.setValue("timeFrameString", m_timeFrameString);
    s.setValue("timeFrameInSeconds", m_timeFrameInSeconds);
    s.setValue("timeFrameIndex", ui->timeFrameComboBox->currentIndex());
    s.setValue("lookback", ui->lookbackSpinBox->value());
    s.setValue("pairsTabWidgetIndex", ui->pairsTabWidget->currentIndex());
    s.beginWriteArray("pairsTabWidgetPages");
    int size = ui->pairsTabWidget->count();
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        s.setValue("tabText", ui->pairsTabWidget->tabText(i));
        if (i == 0)
            s.setValue("showButton1Clicked", m_pair1ShowButtonClickedAlready);
        else
            s.setValue("showButton2Clicked", m_pair2ShowButtonClickedAlready);

        Ui::ContractDetailsWidget* c = NULL;
        if (i==0)
            c = ui->pair1ContractDetailsWidget->getUi();
        else
            c = ui->pair2ContractDetailsWidget->getUi();
        s.setValue("securityTypeComboBoxIndex", c->securityTypeComboBox->currentIndex());
        s.setValue("symbol", c->symbolLineEdit->text());
        s.setValue("exchange", c->exchangeLineEdit->text());
        s.setValue("expiryMonth", c->expiryMonthSpinBox->value());
        s.setValue("expiryYear", c->expiryYearSpinBox->value());
    }
    s.endArray();
    s.beginGroup("mdi");
    s.setValue("viewMode", ui->mdiArea->viewMode());
    s.setValue("mdiCascade", m_mdiCascade);
    s.setValue("mdiTile", m_mdiTile);
    s.beginWriteArray("subwindows");
    size = ui->mdiArea->subWindowList().size();
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        QMdiSubWindow* w = ui->mdiArea->subWindowList().at(i);
        s.setValue("size", w->size());
        s.setValue("position", w->pos());
        
    }
    s.endArray();
    s.endGroup();
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
    s.setValue("overrideUnitSizeCheckBoxState", ui->overrideUnitSizeCheckBox->checkState());
    s.setValue("pair1UnitOverride", ui->pair1UnitOverrideSpinBox->value());
    s.setValue("pair2UnitOverride", ui->pair2UnitOverrideSpinBox->value());
    s.setValue("rsiUpperCheckState", ui->tradeEntryRSIUpperCheckBox->checkState());
    s.setValue("rsiUpper", ui->tradeEntryRSIUpperSpinBox->value());
    s.setValue("percentFromMeanCheckState", ui->tradeEntryPercentFromMeanCheckBox->checkState());
    s.setValue("percentFromMean", ui->tradeEntryPercentFromMeanDoubleSpinBox->value());
    s.setValue("numStdDevLayers", ui->tradeEntryNumStdDevLayersSpinBox->value());
    s.setValue("waitCheckBoxEnabled", ui->waitCheckBox->isEnabled());
    s.setValue("waitCheckBoxState", ui->waitCheckBox->checkState());
    s.setValue("bufferCheckBoxEnabled", ui->layerBufferCheckBox->isEnabled());
    s.setValue("bufferCheckBoxState", ui->layerBufferCheckBox->checkState());
    s.setValue("bufferSpinBoxEnabled", ui->layerBufferDoubleSpinBox->isEnabled());
    s.setValue("buffer", ui->layerBufferDoubleSpinBox->value());
    s.beginWriteArray("layers");
    for (int i=0;i<ui->tradeEntryNumStdDevLayersSpinBox->value();++i) {
        s.setArrayIndex(i);
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
    s.setValue("percentStopLoss", ui->tradeExitPercentStopLossDoubleSpinBox->value());
    s.setValue("percentFromMeanCheckState", ui->tradeExitPercentFromMeanCheckBox->checkState());
    s.setValue("percentFromMean", ui->tradeExitPercentFromMeanDoubleSpinBox->value());
    s.setValue("stdDevCheckBox", ui->tradeExitStdDevCheckBox->checkState());
    s.setValue("stdDev", ui->tradeExitStdDevDoubleSpinBox->value());
    s.endGroup();

    int count = m_securityMap.count();

    if (count == 2) {
        s.beginWriteArray("securities");
        for (int i = 0;i< m_securityMap.count();++i) {
            s.setArrayIndex(i);
            Security* ss = m_securityMap.values().at(i);
            ContractDetails* cd = ss->getContractDetails();
            Contract ct = cd->summary;

            s.beginGroup("contract");
            s.setValue("conId", (int)ct.conId);
            s.setValue("symbol", ct.symbol);
            s.setValue("secType", ct.secType);
            s.setValue("expiry", ct.expiry);
            s.setValue("exchange", ct.exchange);
            s.setValue("primaryExchange", ct.primaryExchange);
            s.setValue("currency", ct.currency);
            s.setValue("localSymbol", ct.localSymbol);
            s.setValue("secId", ct.secId);
            s.endGroup();

            int n = ss->getSecurityOrderMap()->count();
            s.beginWriteArray("orders");
            for (int j=0;j<n;++j) {
                s.setArrayIndex(j);
                long orderId = ss->getSecurityOrderMap()->keys().at(j);
                SecurityOrder* so = ss->getSecurityOrderMap()->values().at(j);
                s.setValue("orderId", (int)orderId);
                s.setValue("status", so->status);
                s.setValue("filled", so->filled);
                s.setValue("remaining", so->remaining);
                s.setValue("avgFillPrice", so->avgFillPrice);
                s.setValue("permId", so->permId);
                s.setValue("parentId", so->parentId);
                s.setValue("lastFillPrice", so->lastFillPrice);
                s.setValue("clientId", so->clientId);
                s.setValue("whyHeld", so->whyHeld);
                s.setValue("totalQuantity", (int)so->order.totalQuantity);
                s.setValue("action", so->order.action);
                s.setValue("commision", so->orderState.commission);
                s.setValue("warningText", so->orderState.warningText);
            }
            s.endArray();
        }
        s.endArray();
    }
}

void PairTabPage::readSettings()
{
    pDebug("1");

    m_readingSettings = true;

    QString sym1, sym2;

    QSettings s;
    s.beginGroup(m_tabSymbol);
    ui->activateButton->setEnabled(s.value("activateButtonEnabled").toBool());
    ui->deactivateButton->setEnabled(s.value("deactivateButtonEnabled").toBool());
    ui->manualTradeEntryCheckBox->setCheckState((Qt::CheckState)s.value("manualTradeEntryCheckBoxState").toInt());
    ui->manualTradeExitCheckBox->setCheckState((Qt::CheckState)s.value("manualTradeExitCheckBoxState").toInt());
    ui->toolBox->setCurrentIndex(s.value("toolBoxIndex").toInt());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/configPage");
    ui->maPeriodSpinBox->setValue(s.value("maPeriod").toInt());
    ui->rsiPeriodSpinBox->setValue(s.value("rsiPeriod").toInt());
    ui->stdDevPeriodSpinBox->setValue(s.value("stdDevPeriod").toInt());
    ui->volatilityPeriodSpinBox->setValue(s.value("volatilityPeriod").toInt());
    s.endGroup();

    s.beginGroup(m_tabSymbol + "/pairsPage");
    m_timeFrame =(TimeFrame) s.value("timeFrame").toInt();
    m_timeFrameString = s.value("timeFrameString").toString();
    m_timeFrameInSeconds = s.value("timeFrameInSeconds").toInt();
    ui->timeFrameComboBox->setCurrentIndex(s.value("timeFrameIndex").toInt());
    ui->lookbackSpinBox->setValue(s.value("lookback").toInt());
    ui->pairsTabWidget->setCurrentIndex(s.value("pairsTabWidgetIndex").toInt());
    int size = s.beginReadArray("pairsTabWidgetPages");
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        ui->pairsTabWidget->setTabText(i, s.value("tabText").toString());

        Ui::ContractDetailsWidget* c = NULL;

        if (i==0)
            c = ui->pair1ContractDetailsWidget->getUi();
        else
            c = ui->pair2ContractDetailsWidget->getUi();
        c->securityTypeComboBox->setCurrentIndex(s.value("securityTypeComboBoxIndex").toInt());
        c->symbolLineEdit->setText(s.value("symbol").toString());
        if (i==0)
            sym1 = s.value("symbol").toString();
        else
            sym2 = s.value("symbol").toString();
        c->exchangeLineEdit->setText(s.value("exchange").toString());
        c->expiryMonthSpinBox->setValue(s.value("expiryMonth").toInt());
        c->expiryYearSpinBox->setValue(s.value("expiryYear").toInt());

        if (!PairTabPage::DontClickShowButtons) {

            bool showButtonClicked = false;

            if (i==0) {
                showButtonClicked = s.value("showButton1Clicked").toBool();
                if (showButtonClicked) {
                    ui->pair1ShowButton->click();
                    m_pair1ShowButtonClickedAlready = true;
                }
            }
            else {
                showButtonClicked = s.value("showButton2Clicked").toBool();
                if (showButtonClicked) {
                    ui->pair2ShowButton->click();
                    m_pair2ShowButtonClickedAlready = true;
                }
            }
        }
    }
    s.endArray();

//    pDebug("2");

    s.beginGroup("mdi");
    ui->mdiArea->setViewMode((QMdiArea::ViewMode)s.value("viewMode").toInt());
    bool isTiled = s.value("mdiTile").toBool();
    bool isCascade = s.value("mdiCascade").toBool();
    if (ui->mdiArea->viewMode() == QMdiArea::SubWindowView) {
        if (isTiled)
            ui->mdiArea->tileSubWindows();
        if (isCascade)
            ui->mdiArea->cascadeSubWindows();
    }
    s.beginWriteArray("subwindows");
    size = ui->mdiArea->subWindowList().size();
    for (int i=0;i<size;++i) {
        s.setArrayIndex(i);
        QMdiSubWindow* w = ui->mdiArea->subWindowList().at(i);
        w->resize(s.value("size").toSize());
        w->move(s.value("position").toPoint());
    }
    s.endArray();
    s.endGroup();
    s.endGroup();

//    pDebug("3");

    s.beginGroup(m_tabSymbol + "/tradeEntry");
    QString accountString = s.value("managedAccountsComboBoxText").toString();

    for (int i=0;i<ui->managedAccountsComboBox->count();++i) {
        if (ui->managedAccountsComboBox->itemText(i) == accountString) {
            ui->managedAccountsComboBox->setCurrentIndex(i);
            break;
        }
    }
    ui->tradeEntryAmountSpinBox->setValue(s.value("amount").toInt());
    ui->overrideUnitSizeCheckBox->setCheckState((Qt::CheckState)s.value("overrideUnitSizeCheckBoxState").toInt());
    ui->pair1UnitOverrideLabel->setText(sym1 + QString(" Units"));
    ui->pair2UnitOverrideLabel->setText(sym2 + QString(" Units"));
    if (ui->overrideUnitSizeCheckBox->isChecked()) {
        ui->pair1UnitOverrideLabel->setVisible(true);
        ui->pair2UnitOverrideLabel->setVisible(true);
        ui->pair1UnitOverrideSpinBox->setVisible(true);
        ui->pair2UnitOverrideSpinBox->setVisible(true);
    }
    else {
        ui->pair1UnitOverrideLabel->setVisible(false);
        ui->pair2UnitOverrideLabel->setVisible(false);
        ui->pair1UnitOverrideSpinBox->setVisible(false);
        ui->pair2UnitOverrideSpinBox->setVisible(false);
    }
    ui->tradeEntryRSIUpperCheckBox->setCheckState((Qt::CheckState)s.value("rsiUpperCheckState").toInt());
    ui->tradeEntryRSIUpperSpinBox->setValue(s.value("rsiUpper").toInt());
    ui->tradeEntryPercentFromMeanCheckBox->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckState").toInt());
    ui->tradeEntryPercentFromMeanDoubleSpinBox->setValue(s.value("percentFromMean").toDouble());
    ui->tradeEntryNumStdDevLayersSpinBox->setValue(s.value("numStdDevLayers").toInt());
    for (int i=0;i<ui->layersTabWidget->count();++i)
        ui->layersTabWidget->removeTab(i);
    ui->waitCheckBox->setEnabled(s.value("waitCheckBoxEnabled").toBool());
    ui->waitCheckBox->setCheckState((Qt::CheckState)s.value("waitCheckBoxState").toInt());
    ui->layerBufferCheckBox->setEnabled(s.value("bufferCheckBoxEnabled").toBool());
    ui->layerBufferCheckBox->setCheckState((Qt::CheckState)s.value("bufferCheckBoxState").toInt());
    ui->layerBufferDoubleSpinBox->setEnabled(s.value("bufferSpinBoxEnabled").toBool());
    ui->layerBufferDoubleSpinBox->setValue(s.value("buffer").toDouble());
    size = s.beginReadArray("layers");

//    qDebug() << "[DEBUG-PairTabPage::readSettings] num layers:" << size;

//    pDebug(4);

    for (int i = 0;i<size;++i) {
        s.setArrayIndex(i);
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
    ui->tradeExitPercentStopLossDoubleSpinBox->setValue(s.value("percentStopLoss").toDouble());
    ui->tradeExitPercentFromMeanCheckBox->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckBoxState").toInt());
    ui->tradeExitPercentFromMeanDoubleSpinBox->setValue(s.value("percentFromMean").toInt());
    ui->tradeExitStdDevCheckBox->setCheckState((Qt::CheckState)s.value("stdDevCheckBox").toDouble());
    ui->tradeExitStdDevDoubleSpinBox->setValue(s.value("stdDev").toDouble());
    s.endGroup();

//    pDebug(6);

//    if (ui->tabWidget->count() > 2) {
//        s.beginGroup(m_tabSymbol);
//        ui->tabWidget->setCurrentIndex(s.value("tabWidgetIndex").toInt());
//        s.endGroup();
//    }
//    if (!ui->pair1ShowButton->isEnabled() && !ui->pair2ShowButton->isEnabled())
    if (m_pair2ShowButtonClickedAlready)
        m_canSetTabWidgetCurrentIndex = true;

    int n1 = s.beginReadArray("securities");
    for (int i = 0;i<n1;++i) {
        s.setArrayIndex(i);

        // A FIX FOR A CORNER CASE... DOES THIS BREAK SOMETHING ELSE?
        if (!m_securityMap.count())
            return;

        int ssSize = m_securityMap.count();
        Q_UNUSED(ssSize);

        Security* ss = m_securityMap.values().at(i);

        int n2 = s.beginReadArray("orders");

        if (n2 == 0) {
            if (ui->manualTradeEntryCheckBox->isChecked()) {
                ui->activateButton->setEnabled(true);
            }
            if (ui->manualTradeExitCheckBox->isChecked()) {
                ui->deactivateButton->setEnabled(false);
            }
        }

        for (int j=0;j<n2;++j) {
            s.setArrayIndex(j);
            int orderId = s.value("orderId").toInt();
            SecurityOrder* so = ss->newSecurityOrder(orderId);
//            so->order.clientId =
            so->order.orderId = orderId;
            so->status = s.value("status").toByteArray();
            so->orderState.status = so->status;
            so->filled = s.value("filled").toInt();
            so->remaining = s.value("remaining").toInt();
            so->avgFillPrice = s.value("avgFillPrice").toDouble();
            so->permId = s.value("permId").toInt();
            so->order.permId = so->permId;
            so->parentId = s.value("parentId").toInt();
            so->lastFillPrice = s.value("lastFillPrice").toDouble();
            so->clientId = s.value("clientId").toInt();
            so->whyHeld = s.value("whyHeld").toByteArray();
            so->order.totalQuantity = s.value("totalQuantity").toInt();
            so->order.action = s.value("action").toByteArray();
            so->orderState.commission = s.value("commision").toDouble();
            so->orderState.warningText = s.value("warningText").toByteArray();

            m_mainWindow->onOrderStatus(so->order.orderId, so->status, so->filled, so->remaining, so->avgFillPrice,
                                            so->permId, so->parentId, so->lastFillPrice, so->clientId, so->whyHeld);
        }
        s.endArray();
    }
    s.endArray();

    m_readingSettings = false;

    pDebug("7");
}

TimeFrame PairTabPage::getTimeFrame() const
{
    return m_timeFrame;
}

QString PairTabPage::getTabSymbol() const
{
    return m_tabSymbol;
}


void PairTabPage::placeOrder(TriggerType triggerType, bool reverse)
{
    if (ui->tradeEntryAmountSpinBox->value() == 0) {
        QMessageBox msgBox;
        msgBox.setText("Can not place order because the amount of money allocated is $0.00");
        msgBox.setInformativeText("Please check configurations and correct the error");
        /*int ret =*/ msgBox.exec();
        return;
    }

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    Contract* c1 = s1->contract();
    Contract* c2 = s2->contract();

    long orderId1 = m_ibClient->getOrderId();
    long orderId2 = m_ibClient->getOrderId();

    SecurityOrder* so1 = s1->newSecurityOrder(orderId1);
    SecurityOrder* so2 = s2->newSecurityOrder(orderId2);

    so1->triggerType = triggerType;
    so2->triggerType = triggerType;

    double last1 = s1->getHistData(m_timeFrame)->close.last();
    double last2 = s2->getHistData(m_timeFrame)->close.last();
    double amount = ui->tradeEntryAmountSpinBox->value();

//qDebug() << "[DEBUG-placeOrder] last1:" << last1 << "last2:" << last2 << "amount:  $" << amount;

    if ( !ui->overrideUnitSizeCheckBox->isChecked()) {

        so1->order.totalQuantity = (long)floor(amount/2/last1);
        so2->order.totalQuantity = (long)floor(amount/2/last2);
    }
    else {
        so1->order.totalQuantity = ui->pair1UnitOverrideSpinBox->value();
        so2->order.totalQuantity = ui->pair2UnitOverrideSpinBox->value();
    }

    if (!reverse)
        so1->order.action = "SELL";
    else
        so1->order.action = "BUY";

//qDebug() << "[DEBUG-placeOrder] so1->order.totalQuantity:" << so1->order.totalQuantity;

    so1->order.orderType = "MKT";
    so1->order.transmit = true;
    so1->order.account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    if (!reverse)
        so2->order.action = "BUY";
    else
        so2->order.action = "SELL";

//qDebug() << "[DEBUG-placeOrder] so2->order.totalQuantity:" << so2->order.totalQuantity;

    so2->order.orderType = "MKT";
    so2->order.transmit = true;
    so2->order.account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    m_ibClient->placeOrder(orderId1, *c1, so1->order);
    m_ibClient->placeOrder(orderId2, *c2, so2->order);

//qDebug() << "[DEBUG-placeOrder] orderId1:" << orderId1 << "orderId2:" << orderId2;


//    Smtp* smtp = new Smtp("vel.accel@gmail.com", "#nec@eEtM4M74gA@", "smtp.gmail.com");
//        connect(smtp, SIGNAL(status(QString)), this, SLOT(onMailSent(QString)));


//    smtp->sendMail("vel.accel@gmail.com", "vel.accel@gmail.com" , "This is a subject","This is a body");

}

void PairTabPage::exitOrder()
{
//qDebug() << "[DEBUG-PairTabPage::exitOrder";

//    Security* s1 = m_securityMap.values().at(0);
//    Security* s2 = m_securityMap.values().at(1);

    for (int i=0;i<2;++i) {
        Security* s = m_securityMap.values().at(i);
        for (int j=0;j<s->getSecurityOrderMap()->count();++j) {
            SecurityOrder* so = s->getSecurityOrderMap()->values().at(j);
            if (so->triggerType == EXIT)
                continue;
            long orderId = m_ibClient->getOrderId();
            SecurityOrder* newSo = s->newSecurityOrder(orderId);
            newSo->triggerType = EXIT;
            newSo->referenceOrderId = so->order.orderId;
            Contract* contract = s->contract();
//            QMap<long, SecurityOrder*>* soMap;


//            if (i==0) {
//                soMap = s1->getSecurityOrderMap();
//                so = soMap->values().at(0);
//                newSo = s1->newSecurityOrder(orderId);
//                contract = s1->contract();
//            }
//            else {
//                so = s2->getSecurityOrderMap()->values().at(0);
//                newSo = s2->newSecurityOrder(orderId);
//                contract = s2->contract();
//            }

            if (so->order.action == "BUY")
                newSo->order.action = "SELL";
            else if (so->order.action == "SELL")
                newSo->order.action = "BUY";

            newSo->order.transmit = true;
            newSo->order.account = so->order.account;
            newSo->order.orderType = so->order.orderType;
            newSo->order.totalQuantity = so->order.totalQuantity;

            m_ibClient->placeOrder(orderId, *contract, newSo->order);


//        Order o1 = s1->getSecurityOrderMap()->values().at(i)->order;
//        Order o2 = s2->getSecurityOrderMap()->values().at(i)->order;

//        if (o1.action == "BUY") {
//            o1.action = "SELL";
//            o2.action = "BUY";
//        }
//        else {
//            o1.action = "BUY";
//            o2.action = "SELL";
//        }
//        int cnt1 = s1->getSecurityOrderMap()->count();
//        int cnt2 = s2->getSecurityOrderMap()->count();
//        for (int j=0;j<cnt1 ;++j) {
//            m_ibClient->placeOrder(s1->getSecurityOrderMap()->keys().at(i), *s1->contract(), o1);
//        }
//        for (int j=0;j<cnt2;++j) {
//            m_ibClient->placeOrder(s2->getSecurityOrderMap()->keys().at(j), *s2->contract(), o2);
//        }
        }
    }
}

void PairTabPage::showPlot(long tickerId)
{
    Security* s = m_securityMap.value(tickerId);
    DataVecsHist* dvh = s->getHistData(m_timeFrame);

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh->timeStamp, dvh->close);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle(s->contract()->symbol);
    sw->maximumSize();
    cp->show();
}


void PairTabPage::appendPlotsAndTable(long sid)
{
//    QString sidString("sid: " + QString::number(sid));
    // pDebug(sidString);

    bool isS1 = false;
    bool isS2 = false;

    Security* s = m_securityMap.value(sid);

    if (s == m_securityMap.values().at(0))
        isS1 = true;
    if (m_securityMap.count() == 2 && s == m_securityMap.values().at(1))
        isS2 = true;

    // pDebug(isS1);
    // pDebug(isS2);

//    bool isS1 = m_securityMap.keys().indexOf(sid) == 0;
//    bool isS2 = m_securityMap.keys().indexOf(sid) == 1;

    DataVecsHist* dvh = s->getHistData(m_timeFrame);
    DataVecsRaw*  dvr = s->getRawData();
    DataVecsRaw*  dvr1 = NULL;
    DataVecsRaw*  dvr2 = NULL;

    if (!dvh || dvh->timeStamp.isEmpty()) {
        // pDebug("return");
        return;
    }
    s->setLastBarsTimeStamp(dvh->timeStamp.last());

    QCustomPlot* cp = m_customPlotMap.value(this->getPlotIndexFromSymbol(s));

    double timeStampLast = 0;
    double closeLast     = 0;

    bool dvrTimeStampIsEmpty = true;

    // pDebug("1a");

    if (dvr) {
        // pDebug("2");
        dvrTimeStampIsEmpty = dvr->timeStamp.isEmpty();
    }

    if (!dvr || dvrTimeStampIsEmpty) {
//         pDebug("3");
        timeStampLast = dvh->timeStamp.last();
        closeLast = dvh->close.last();
    }
    if (dvr && !dvrTimeStampIsEmpty) {
//         pDebug("4");
        timeStampLast = dvh->timeStamp.last() + m_timeFrameInSeconds;
        closeLast = dvr->price.last();
    }

    QCPDataMap* dataMap = cp->graph()->data();
    //dataMap->remove(dataMap->keys().last());
    dataMap->insert(timeStampLast, QCPData(timeStampLast, closeLast));

    // pDebug("5");

    if (ui->autoUpdateRangeCheckBox->isChecked())
        cp->xAxis->setRangeUpper(timeStampLast);
    cp->replot();

    Ui::DataToolBoxWidget* w = ui->chartDataPage->getUi();
    QTableWidget* tw = mwui->homeTableWidget;


    if (isS1) {
        // pDebug("isS1"); // pDebug(s);
        // pDebug(QString::number(closeLast,'f',2));
        w->lastPair1PriceLineEdit->setText(QString::number(closeLast));
    }
    if (isS2) {
        // pDebug("isS2"); // pDebug(s);
        // pDebug(QString::number(closeLast));
        w->lastPair2PriceLineEdit->setText(QString::number(closeLast,'f',2));
    }

    if (m_securityMap.size() == 1) {
        // pDebug("6");
        return;
    }

    if (m_bothPairsUpdated) {
        // pDebug("7");
        if (m_securityMap.size() > 1 && m_securityMap.values().at(1) == s) {
            m_bothPairsUpdated = false;
            // pDebug("returning early");
            return;
        }
    }

    // pDebug("8");

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    DataVecsHist* dvh1 = s1->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = s2->getHistData(m_timeFrame);

    // pDebug(9);

    if (!dvh1 || !dvh2)
        return;

    // pDebug(10);

    dvr1 = s1->getRawData();
    dvr2 = s2->getRawData();

    QVector<double> timeStampVec = dvh1->timeStamp;
    QVector<double> closeVec1 = dvh1->close;
    QVector<double> closeVec2 = dvh2->close;
    QVector<double> highVec1  = dvh1->high;
    QVector<double> highVec2  = dvh2->high;
    QVector<double> lowVec1   = dvh1->low;
    QVector<double> lowVec2   = dvh2->low;

    // pDebug(11);

    if (dvr1 && !dvr1->price.isEmpty()) {
        // pDebug("");
        timeStampVec.append(dvh1->timeStamp.last() + m_timeFrameInSeconds);
        closeVec1.append(dvr1->price.last());
        highVec1.append(s1->getRawPriceHigh());
        lowVec1.append(s1->getRawPriceLow());
    }

    // pDebug(12);

    if (dvr2 && !dvr2->price.isEmpty()) {
        timeStampVec.append(dvh1->timeStamp.last() + m_timeFrameInSeconds);
        closeVec2.append(dvr2->price.last());
        highVec2.append(s2->getRawPriceHigh());
        lowVec2.append(s2->getRawPriceLow());
    }

    // pDebug("");

    m_ratio = getRatio(closeVec1, closeVec2);

    m_ratioMA = getMA(m_ratio, ui->maPeriodSpinBox->value());
    m_ratioStdDev = getStdDevVector(m_ratio, ui->stdDevPeriodSpinBox->value());
    m_ratioPercentFromMA = getPercentFromMA(m_ratio, ui->maPeriodSpinBox->value());
    m_correlation = getCorrelation(dvh1->close, dvh2->close);
//    m_ratioVolatility = getRatioVolatility(getRatio(dvh1->high,dvh2->high), getRatio(dvh1->low,dvh2->low), ui->volatilityPeriodSpinBox->value());
    m_ratioVolatility = getRatioVolatility(
                getRatio(getDiff(highVec1, lowVec1), getDiff(highVec2, lowVec2)),
                ui->volatilityPeriodSpinBox->value());
    m_ratioRSI = getRSI(m_ratio, ui->rsiPeriodSpinBox->value());
    int period = ui->rsiSpreadSpinBox->value();
    m_pair1RSI = getRSI(closeVec1, period);
    m_pair2RSI = getRSI(closeVec2, period);
    m_rsiSpread = getDiff(m_pair1RSI, m_pair2RSI);

    // update chart data page
    w->timeLabel->setText(QDateTime::fromTime_t((uint)timeStampVec.last()).time().toString("'Timestamp:    ' h:mm:ss AP"));
    w->lastCorrelationLineEdit->setText(QString::number(m_correlation.last(),'f',2));
    w->lastMaLineEdit->setText(QString::number(m_ratioMA.last(),'f',2));
    w->lastPcntFromMaLineEdit->setText(QString::number(m_ratioPercentFromMA.last(),'f',2));
    w->lastRatioLineEdit->setText(QString::number(m_ratio.last(),'f',2));
    w->lastRsiOfRatioLineEdit->setText(QString::number(m_ratioRSI.last(),'f',2));
    w->lastRsiSpreadLineEdit->setText(QString::number(m_rsiSpread.last(),'f',2));
    w->lastStdDevLineEdit->setText(QString::number(m_ratioStdDev.last(),'f',2));
    w->lastVolatilityLineEdit->setText(QString::number(m_ratioVolatility.last(),'f',2));

    // pDebug("");

    double ts = dvh->timeStamp.last();

//    pDebug(QDateTime::fromTime_t((uint)ts));

    double min = 0;
    double max = 0;

    for (int i=0;i<ui->mdiArea->subWindowList().size();++i) {

        QMdiSubWindow* w = ui->mdiArea->subWindowList().at(i);
        QString tabText = w->windowTitle();
        cp = qobject_cast<QCustomPlot*>(w->widget());
        dataMap = cp->graph(0)->data();

        if (tabText == "Ratio") {

//            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratio.last()));

            dataMap = cp->graph(1)->data();
//            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratioMA.last()));

            min = getMin(m_ratio);
            max = getMax(m_ratio);
            cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

        }
        else if (tabText == "RatioStdDev") {

//            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratioStdDev.last()));
            min = getMin(m_ratioStdDev);
            max = getMax(m_ratioStdDev);
            cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

        }
        else if (tabText == "PcntFromRatioMA") {

            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratioPercentFromMA.last()));
            min = getMin(m_ratioPercentFromMA);
            max = getMax(m_ratioPercentFromMA);
            cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

        }
        else if (tabText == "Correlation") {

            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_correlation.last()));
            min = getMin(m_correlation);
            max = getMax(m_correlation);
            cp->yAxis->setRange(-1.0,1.0);

        }
        else if (tabText == "RatioVolatility") {

            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratioVolatility.last()));
            min = getMin(m_ratioVolatility);
            max = getMax(m_ratioVolatility);
            cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

        }
        else if (tabText == "RatioRSI") {

            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_ratioRSI.last()));
            min = getMin(m_ratioRSI);
            max = getMax(m_ratioRSI);
            cp->yAxis->setRange(0,100);

        }
        else if (tabText == "RSISpread") {

            //dataMap->remove(dataMap->keys().last());
            dataMap->insert(ts, QCPData(ts, m_rsiSpread.last()));
            min = getMin(m_rsiSpread);
            max = getMax(m_rsiSpread);
            cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));
        }

        if (ui->autoUpdateRangeCheckBox->isChecked())
            cp->xAxis->setRangeUpper(timeStampLast);

        cp->replot();
    }

    // pDebug("");

    int row = -1;

    for (int r=0;r<tw->rowCount();++r) {
        bool isTheRow = false;
        for (int c=0;c<tw->columnCount();++c) {
            QTableWidgetItem* hItem = tw->horizontalHeaderItem(c);
            QString hItemText = hItem->text();
            QTableWidgetItem* item = tw->item(r,c);
            QString itemText = item->text();

//            m_headerLabels << "Pair"
//                    << "Price1"
//                    << "Price2"
//                    << "Ratio"
//                    << "RatioMA"
//                    << "RatioStdDev"
//                    << "PcntFromRatioMA"
//                    << "Correlation"
//                    << "RatioVolatility"
//                    << "RatioRSI"
//                    << "SpreadRSI";

            if (hItemText == "Pair") {
                if (itemText == m_tabSymbol) {
                    isTheRow = true;
                    break;
                }
//              continue;
            }
        }
        if (isTheRow) {
            row = r;
        }
//        else {
//            // pDebug("FUCK FUCK FUCK");
//        }
    }

    // pDebug("");

    if (row == -1) {
        // pDebug("row == -1");
        return;
    }

    // pDebug("");

    for (int c=0;c<tw->columnCount();++c) {
        QTableWidgetItem* headerItem = tw->horizontalHeaderItem(c);
        QString headerItemText = headerItem->text();
        QTableWidgetItem* item = tw->item(row, c);
        if (!item)
            continue;
        if (headerItemText == "Price1") {
            if (s == s1) {
                item->setText(QString::number(closeVec1.last(), 'f', 2));
            }
        }
        else if (headerItemText == "Price2") {
            if (s == s2) {
                tw->item(row, c)->setText(QString::number(closeVec2.last(), 'f', 2));
            }
        }
        if (headerItemText == "Ratio")
            item->setText(QString::number(m_ratio.last(),'f',2));
        else if (headerItemText == "RatioMA")
            item->setText(QString::number(m_ratioMA.last(),'f',2));
        else if (headerItemText == "RatioStdDev")
            item->setText(QString::number(m_ratioStdDev.last(),'f',2));
        else if (headerItemText == "PcntFromRatioMA")
            item->setText(QString::number(m_ratioPercentFromMA.last(),'f',2));
        else if (headerItemText == "Correlation")
            item->setText(QString::number(m_correlation.last(),'f',2));
        else if (headerItemText == "RatioVolatility")
            item->setText(QString::number(m_ratioVolatility.last(),'f',2));
        else if (headerItemText == "RatioRSI")
            item->setText(QString::number(m_ratioRSI.last(),'f',2));
        else if (headerItemText == "SpreadRSI")
            item->setText(QString::number(m_rsiSpread.last(),'f',2));
    }

    tw->update();

    m_bothPairsUpdated = true;

    //    qDebug() << "[DEBUG-appendPlotsAndTable]" << "leaving";
    // pDebug("leaving");
}




void PairTabPage::plotRatio()
{
    Security* s1 = NULL;
    Security* s2 = NULL;
    DataVecsHist* dvh1 = NULL;
    DataVecsHist* dvh2 = NULL;

    s1 = m_securityMap.values().at(0);
    s2 = m_securityMap.values().at(1);

    dvh1 = s1->getHistData(m_timeFrame);
    dvh2 = s2->getHistData(m_timeFrame);

    QCustomPlot* cp = createPlot();
    QVector<double> ts;

    if (dvh1->timeStamp.size() < dvh2->timeStamp.size()) {
        int mid = dvh2->timeStamp.size() - dvh1->timeStamp.size();
        m_ratio = getRatio(dvh1->close, dvh2->close.mid(mid));
        ts = dvh1->timeStamp;
    }
    else {
        int mid = dvh1->timeStamp.size() - dvh2->timeStamp.size();
        m_ratio = getRatio(dvh1->close.mid(mid), dvh2->close);
        ts = dvh2->timeStamp;
    }

//qDebug() << m_ratio;

    int period = qMin(ui->maPeriodSpinBox->value(), qMin(dvh1->timeStamp.size(), dvh2->timeStamp.size()));
    m_ratioMA = getMA(m_ratio, period);

//    qDebug() << m_ratioMA;

//    qDebug() << getExpMA(m_ratio, period);
//    qApp->exit();

    int diff;

    diff = ts.size() - m_ratioMA.size();

    // Ratio
    addGraph(cp, ts.mid(diff), m_ratio.mid(diff));
//    addGraph(cp, ts, m_ratio);

    // MA
    addGraph(cp, ts.mid(diff), m_ratioMA, Qt::red, false);


    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("Ratio");
    cp->setToolTip("Ratio/RatioMA");
    sw->maximumSize();
    cp->show();
}




void PairTabPage::plotRatioMA()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioMA = getMA(m_ratio, ui->maPeriodSpinBox->value());

    int diff = dvh1->timeStamp.size() - m_ratioMA.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioMA);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RatioMA");
    sw->maximumSize();
    cp->show();
}


void PairTabPage::plotRatioStdDev()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    int period = qMin(ui->stdDevPeriodSpinBox->value(), m_ratio.size());

    m_ratioStdDev = getStdDevVector(m_ratio, period);

    int diff = dvh1->timeStamp.size() - m_ratioStdDev.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioStdDev);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RatioStdDev");
    sw->maximumSize();
    cp->show();
}


void PairTabPage::plotRatioPercentFromMean()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioPercentFromMA = getPercentFromMA(m_ratio, ui->maPeriodSpinBox->value());

    int diff = dvh1->timeStamp.size() - m_ratioPercentFromMA.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioPercentFromMA);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("PcntFromRatioMA");
    sw->maximumSize();
    cp->show();
}


void PairTabPage::plotCorrelation()
{
//    P_DEBUG;
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    m_correlation = getCorrelation(dvh1->close, dvh2->close);

    int diff = dvh1->timeStamp.size() - m_correlation.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_correlation);
    cp->yAxis->setRange(-1.0, 1.0);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("Correlation");
    sw->maximumSize();
    cp->show();

//    P_DEBUG;
}

void PairTabPage::plotCointegration()
{
    for (int i=0;i<250;++i) {
        m_cointegration[i] = 0;
    }
}

void PairTabPage::plotRatioVolatility()
{
    pDebug("");

    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    int period = qMin(ui->volatilityPeriodSpinBox->value(), m_ratio.size());

//    m_ratioVolatility = getRatioVolatility(getRatio(dvh1->high,dvh2->high), getRatio(dvh1->low,dvh2->low), period);
    QVector<double> d1 = getDiff(dvh1->high,dvh1->low);
    QVector<double> d2  = getDiff(dvh2->high,dvh2->low);
    QVector<double> r   = getRatio(d1,d2);
    m_ratioVolatility = getRatioVolatility(r, period);

    int diff = dvh1->timeStamp.size() - m_ratioVolatility.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioVolatility);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RatioVolatility");
    sw->maximumSize();
    cp->show();

    pDebug("leaving");
}

void PairTabPage::plotRatioRSI()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    int period = qMin(ui->volatilityPeriodSpinBox->value(), m_ratio.size());

    m_ratioRSI = getRSI(m_ratio, period);

    int diff = dvh1->timeStamp.size() - m_ratioRSI.size();

    QCustomPlot* cp = createPlot();

    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioRSI);
    cp->yAxis->setRange(0, 100);


    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RatioRSI");
    sw->maximumSize();
    cp->show();
}

void PairTabPage::plotRSISpread()
{
    Security* s1 = NULL;
    Security* s2 = NULL;
    DataVecsHist* dvh1 = NULL;
    DataVecsHist* dvh2 = NULL;

    for (int i=0;i<m_securityMap.count();++i) {
        long key = m_securityMap.keys().at(i);
        Security* s = m_securityMap.values().at(i);
        if (key == 0 && s == NULL)
            m_securityMap.remove(key);
    }

    s1 = m_securityMap.values().at(0);
    s2 = m_securityMap.values().at(1);

    dvh1 = s1->getHistData(m_timeFrame);
    dvh2 = s2->getHistData(m_timeFrame);

    int period = ui->rsiSpreadSpinBox->value();

    m_pair1RSI = getRSI(dvh1->close, period);
    m_pair2RSI = getRSI(dvh2->close, period);
    m_rsiSpread = getDiff(m_pair1RSI, m_pair2RSI);

    int diff = 0;
    QVector<double> ts;

    if (dvh1->timeStamp.size() < dvh2->timeStamp.size()) {
        diff = dvh1->timeStamp.size() - m_rsiSpread.size();
        ts = dvh1->timeStamp.mid(diff);
    }
    else {
        diff = dvh2->timeStamp.size() - m_rsiSpread.size();
        ts = dvh2->timeStamp.mid(diff);
    }
    QString tsLast = QDateTime::fromTime_t((int)ts.last()).toString("yyMMdd::hh:mm:ss");
    QCustomPlot* cp = createPlot();
    addGraph(cp, ts, m_rsiSpread);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RSISpread");
    sw->maximumSize();
    cp->show();
}



void PairTabPage::addTableRow()
{
//    qDebug() << "[DEBUG-addTableRow]";

    QTableWidget* tab = mwui->homeTableWidget;

    for (int r=0;r<tab->rowCount();++r) {
        if (tab->item(r,0)->text() == m_tabSymbol)
            tab->removeRow(r);
    }

    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    DataVecsHist* d1 = s1->getHistData(m_timeFrame);
    DataVecsHist* d2 = s2->getHistData(m_timeFrame);

//    m_headerLabels << "Pair"
//            << "Price2"
//            << "Ratio"
//            << "RatioMA"
//            << "RatioStdDev"
//            << "PcntFromRatioMA"
//            << "Correlation"
//            << "RatioVolatility"
//            << "RatioRSI"
//            << "RSISpread;



//    QString sym1 = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text();
//    QString sym2 = m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text();

//    QString expiryString1 = QString::number(m_pair1ContractDetailsWidget->getUi()->expiryMonthSpinBox->value())
//            + "/"
//            + QString::number(m_pair1ContractDetailsWidget->getUi()->expiryYearSpinBox->value());

//    QString expiryString2 = QString::number(m_pair2ContractDetailsWidget->getUi()->expiryMonthSpinBox->value())
//            + "/"
//            + QString::number(m_pair2ContractDetailsWidget->getUi()->expiryYearSpinBox->value());


//    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
//        sym1 += " [" + expiryString1 + "]";
//    }
//    if (m_pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
//        sym2 += " [" + expiryString2 + "]";
//    }

    QStringList itemList;
    itemList /*<< sym1
             << sym2
             << m_timeFrameString*/
             << m_tabSymbol
             << QString::number(d1->close.last(),'f',2)
             << QString::number(d2->close.last(),'f',2)
             << QString::number(m_ratio.last(),'f',2)
             << QString::number(m_ratioMA.last(),'f',2)
             << QString::number(m_ratioStdDev.last(),'f',2)
             << QString::number(m_ratioPercentFromMA.last(),'f',2)
             << QString::number(m_correlation.last(),'f',2)
             << QString::number(m_ratioVolatility.last(),'f',2)
             << QString::number(m_ratioRSI.last(),'f',2)
             << QString::number(m_rsiSpread.last(),'f',2);
                ;

//    qDebug() << "itemList:" << itemList;
    int numRows = tab->rowCount();

//qDebug() << "[DEBUG-addTableRow] numRows:" << numRows;

    if (numRows == 0)
        m_homeTablePageRowIndex = 0;
    else
        m_homeTablePageRowIndex = numRows;
    tab->setRowCount(m_homeTablePageRowIndex + 1);
    for (int i=0;i<itemList.size();++i) {
        TableWidgetItem* item = new TableWidgetItem(itemList.at(i));
        tab->setItem(m_homeTablePageRowIndex, i, item);
    }
    tab->update();

    /// update chart data page
//    DataToolBoxWidget* d = ui->
    Ui::DataToolBoxWidget* w = ui->chartDataPage->getUi();
    w->lastPair1PriceLineEdit->setText(QString::number(d1->close.last(),'f',2));
    w->lastPair2PriceLineEdit->setText(QString::number(d2->close.last(),'f',2));
    w->timeLabel->setText(QDateTime::fromTime_t((uint)d1->timeStamp.last()).time().toString("'Timestamp:    ' h:mm:ss AP"));
    w->lastCorrelationLineEdit->setText(QString::number(m_ratioStdDev.last(),'f',2));
    w->lastMaLineEdit->setText(QString::number(m_ratioMA.last(),'f',2));
    w->lastPcntFromMaLineEdit->setText(QString::number(m_ratioPercentFromMA.last(),'f',2));
    w->lastRatioLineEdit->setText(QString::number(m_ratio.last(),'f',2));
    w->lastRsiOfRatioLineEdit->setText(QString::number(m_ratioRSI.last(),'f',2));
    w->lastRsiSpreadLineEdit->setText(QString::number(m_rsiSpread.last(),'f',2));
    w->lastStdDevLineEdit->setText(QString::number(m_ratioStdDev.last(),'f',2));
    w->lastVolatilityLineEdit->setText(QString::number(m_ratioVolatility.last(),'f',2));

//    qDebug() << "[DEBUG-addTableRow] leaving";
}

void PairTabPage::checkTradeTriggers()
{
    int numLayers = ui->tradeEntryNumStdDevLayersSpinBox->value();
    bool rsiUpperChecked = ui->tradeEntryRSIUpperCheckBox->isChecked();
//    bool rsiLowerChecked = ui->tradeEntryRSILowerCheckBox->isChecked();
    bool pcntFromMeanChecked = ui->tradeEntryPercentFromMeanCheckBox->isChecked();

    if (!m_ratioRSITriggerActivated && rsiUpperChecked) {
        double lastRSI = m_ratioRSI.last();
        if (lastRSI > ui->tradeEntryRSIUpperSpinBox->value()) {
            if (numLayers == 0)
                placeOrder(RSI);
            m_ratioRSITriggerActivated = true;
        }
        else if (lastRSI < ui->tradeEntryRSILowerSpinBox->value()) {
            if (numLayers == 0)
                placeOrder(RSI, true);
            m_ratioRSITriggerActivated = true;
        }
    }

    if (!m_percentFromMeanTriggerActivated && ui->tradeEntryPercentFromMeanCheckBox->checkState() == Qt::Checked) {
        double lastPofM = m_ratioPercentFromMA.last();
        if (fabs(lastPofM) > ui->tradeEntryPercentFromMeanDoubleSpinBox->value()) {
            if (numLayers == 0) {
                if (lastPofM > 0)
                    placeOrder(PCNT);
                else if (lastPofM < 0)
                    placeOrder(PCNT, true);
            }
            m_percentFromMeanTriggerActivated = true;
        }
    }

    if (ui->tradeEntryNumStdDevLayersSpinBox->value() > 0) {

        double lastStdDev = m_ratioStdDev.last();
        bool reverse = false;
        if (lastStdDev < 0) {
            lastStdDev = fabs(lastStdDev);
            reverse = true;
        }
        bool wait = false;

        for (int i=0;i<numLayers;++i) {
            if (lastStdDev > m_stdDevLayerPeaks.at(i))
                m_stdDevLayerPeaks[i] = lastStdDev;
            StdDevLayerTab* t = qobject_cast<StdDevLayerTab*>(ui->layersTabWidget->widget(i));
            double stdDevVal = t->getUi()->layerStdDevDoubleSpinBox->value();
            double peak = m_stdDevLayerPeaks.at(i);
            double buffVal = 0;
            double trailVal = 0;

            if (ui->layerBufferCheckBox->checkState() == Qt::Checked) {
                buffVal = ui->layerBufferDoubleSpinBox->value();
            }

            if (t->getUi()->layerTrailCheckBox->checkState() == Qt::Checked) {
                trailVal = t->getUi()->layerTrailDoubleSpinBox->value();
            }

            if (!trailVal && ui->waitCheckBox->checkState() == Qt::Checked) {
                wait = true;
            }

            // Now check the strategies

            if (trailVal) {
                if (t->getUi()->layerStdMinCheckBox->checkState() == Qt::Checked) {
                    double stdMin = t->getUi()->layerStdMinDoubleSpinBox->value();
                    if (peak > stdMin && lastStdDev < (peak - trailVal)) {
                        if (!pcntFromMeanChecked
                                && !rsiUpperChecked) {
                            placeOrder((TriggerType)i, reverse);
                        }
                        else if (pcntFromMeanChecked
                                 && !rsiUpperChecked
                                 && m_percentFromMeanTriggerActivated) {
                                placeOrder((TriggerType)i, reverse);
                        }
                        else if(!pcntFromMeanChecked
                                && rsiUpperChecked
                                && m_ratioRSITriggerActivated) {
                            placeOrder((TriggerType)i, reverse);
                        }
                    }
                }
                else if (lastStdDev < (peak - trailVal)) {
                    if (!pcntFromMeanChecked
                            && !rsiUpperChecked) {
                        placeOrder((TriggerType)i, reverse);
                    }
                    else if (pcntFromMeanChecked
                             && !rsiUpperChecked
                             && m_percentFromMeanTriggerActivated) {
                            placeOrder((TriggerType)i, reverse);
                    }
                    else if(!pcntFromMeanChecked
                            && rsiUpperChecked
                            && m_ratioRSITriggerActivated) {
                        placeOrder((TriggerType)i, reverse);
                    }
                }
            }
            if (wait) {
                if (i == (numLayers - 1)
                         && ui->waitCheckBox->checkState() == Qt::Checked
                         && peak > stdDevVal
                         && lastStdDev < stdDevVal - buffVal) // if bufVal is zero its like its not being used
                {
                    for (int i=0;i<numLayers;++i) {
                        if (!pcntFromMeanChecked
                                && !rsiUpperChecked) {
                            placeOrder((TriggerType)i, reverse);
                        }
                        else if (pcntFromMeanChecked
                                 && !rsiUpperChecked
                                 && m_percentFromMeanTriggerActivated) {
                                placeOrder((TriggerType)i, reverse);
                        }
                        else if(!pcntFromMeanChecked
                                && rsiUpperChecked
                                && m_ratioRSITriggerActivated) {
                            placeOrder((TriggerType)i, reverse);
                        }
                    }
                }
            }

            if (!wait && !trailVal && lastStdDev > stdDevVal) {
                if (!pcntFromMeanChecked
                        && !rsiUpperChecked) {
                    placeOrder((TriggerType)i, reverse);
                }
                else if (pcntFromMeanChecked
                         && !rsiUpperChecked
                         && m_percentFromMeanTriggerActivated) {
                        placeOrder((TriggerType)i, reverse);
                }
                else if(!pcntFromMeanChecked
                        && rsiUpperChecked
                        && m_ratioRSITriggerActivated) {
                    placeOrder((TriggerType)i, reverse);
                }
            }
        }
    }
}

void PairTabPage::checkTradeExits()
{
    if (ui->tradeExitPercentStopLossCheckBox->checkState() == Qt::Checked) {

        // TODO: AFTER ORDERS AND PRICE OF ORDER IS DONE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }

    if (ui->tradeExitPercentFromMeanCheckBox->checkState() == Qt::Checked) {
        double lastPercentFromMean = m_ratioPercentFromMA.last();
        double trigger = ui->tradeExitPercentFromMeanDoubleSpinBox->value();
        if (lastPercentFromMean < trigger)
            exitOrder();

    }

    if (ui->tradeExitStdDevCheckBox->checkState() == Qt::Checked) {
        double lastStdDev = fabs(m_ratioStdDev.last());
        double trigger = ui->tradeExitStdDevDoubleSpinBox->value();
        if (lastStdDev < trigger)
            exitOrder();
    }
}



int PairTabPage::getPlotIndexFromSymbol(Security* s)
{
    QList<QMdiSubWindow*> l = ui->mdiArea->subWindowList();
    int ret = -1;
    for (int i=0;i<l.size();++i) {
        QMdiSubWindow* w = l.at(i);
        if (w->windowTitle() == s->contract()->symbol)
            ret = i;
    }
    return ret;
}

void PairTabPage::setDefaults()
{
    GlobalConfigDialog* d = m_mainWindow->getGlobalConfigDialog();
    Ui::GlobalConfigDialog* dui = d->getUi();
    ui->timeFrameComboBox->setCurrentIndex(dui->timeFrameComboBox_2->currentIndex());
    ui->maPeriodSpinBox->setValue(dui->maPeriodSpinBox->value());
    ui->rsiPeriodSpinBox->setValue(dui->rsiPeriodSpinBox->value());
    ui->stdDevPeriodSpinBox->setValue(dui->stdDevPeriodSpinBox->value());
    ui->volatilityPeriodSpinBox->setValue(dui->volatilityPeriodSpinBox->value());
    ui->rsiSpreadSpinBox->setValue(dui->rsiSpreadSpinBox->value());

    QString accountString = dui->managedAccountsComboBox_2->currentText();

    for (int i=0;i<m_managedAccounts.size();++i) {
        ui->managedAccountsComboBox->addItem(m_managedAccounts.at(i));
    }
    for (int i=0;i<ui->managedAccountsComboBox->count();++i) {
        if (m_managedAccounts.at(i) == accountString) {
            ui->managedAccountsComboBox->setCurrentIndex(i);
            break;
        }
    }

    ui->tradeEntryAmountSpinBox->setValue(dui->tradeEntryAmountSpinBox_2->value());
    ui->tradeEntryRSILowerCheckBox->setChecked(dui->tradeEntryRSILowerCheckBox_3->isChecked());
    ui->tradeEntryRSILowerSpinBox->setValue(dui->tradeEntryRSILowerSpinBox_3->value());
    ui->tradeEntryRSIUpperCheckBox->setChecked(dui->tradeEntryRSIUpperCheckBox_2->isChecked());
    ui->tradeEntryRSIUpperSpinBox->setValue(dui->tradeEntryRSIUpperSpinBox_2->value());
    ui->tradeEntryPercentFromMeanCheckBox->setChecked(dui->tradeEntryPercentFromMeanCheckBox_2->isChecked());
    ui->tradeEntryPercentFromMeanDoubleSpinBox->setValue(dui->tradeEntryPercentFromMeanDoubleSpinBox_2->value());
    ui->tradeEntryNumStdDevLayersSpinBox->setValue(dui->tradeEntryNumStdDevLayersSpinBox_2->value());
    ui->waitCheckBox->setChecked(dui->waitCheckBox_2->isChecked());
    ui->layerBufferCheckBox->setChecked(dui->layerBufferCheckBox_2->isChecked());
    ui->layerBufferDoubleSpinBox->setValue(dui->layerBufferDoubleSpinBox_2->value());
    for (int i = 0;i<ui->tradeEntryNumStdDevLayersSpinBox->value();++i) {
        StdDevLayerTab* t = new StdDevLayerTab(i, ui->layersTabWidget);
        ui->layersTabWidget->addTab(t, QString::number(i));
        t->getUi()->layerStdDevDoubleSpinBox->setValue(dui->layerStdDevDoubleSpinBox->value());
        t->getUi()->layerTrailCheckBox->setChecked(dui->layerTrailCheckBox->isChecked());
        t->getUi()->layerTrailDoubleSpinBox->setValue(dui->layerTrailDoubleSpinBox->value());
        t->getUi()->layerStdMinCheckBox->setChecked(dui->layerStdMinCheckBox->isChecked());
        t->getUi()->layerStdMinDoubleSpinBox->setValue(dui->layerStdMinDoubleSpinBox->value());
    }
    ui->tradeExitPercentStopLossCheckBox->setChecked(dui->tradeExitPercentStopLossCheckBox_2->isChecked());
    ui->tradeExitPercentStopLossDoubleSpinBox->setValue(dui->tradeExitPercentStopLossDoubleSpinBox_2->value());
    ui->tradeExitPercentFromMeanCheckBox->setChecked(dui->tradeExitPercentFromMeanCheckBox_2->isChecked());
    ui->tradeExitPercentFromMeanDoubleSpinBox->setValue(dui->tradeExitPercentFromMeanDoubleSpinBox_2->value());
    ui->tradeExitStdDevCheckBox->setChecked(dui->tradeExitStdDevCheckBox_2->isChecked());
    ui->tradeExitStdDevDoubleSpinBox->setValue(dui->tradeExitStdDevDoubleSpinBox_2->value());
    ui->autoUpdateRangeCheckBox->setChecked(dui->autoUpdateRangeCheckBox->isChecked());
}

QCustomPlot *PairTabPage::createPlot()
{
    QCustomPlot* cp = new QCustomPlot();

    cp->setCursor(QCursor(Qt::CrossCursor));

    connect(cp, SIGNAL(plottableDoubleClick(QCPAbstractPlottable*,QMouseEvent*)),
            this, SLOT(onCustomPlotDoubleClick(QCPAbstractPlottable*,QMouseEvent*)));
    connect(cp, SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(onMouseMove(QMouseEvent*)));

//    cp->setMinimumSize(m_minWidth, m_minHeight);
    m_customPlotMap[ui->mdiArea->subWindowList().size()] = cp;

    cp->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(cp, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequest(QPoint)));

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
    return cp;
}

QCPGraph *PairTabPage::addGraph(QCustomPlot *cp, QVector<double> x, QVector<double> y, QColor penColor, bool useBrush)
{
    cp->addGraph();
    int count = cp->graphCount();
    QCPGraph* g = cp->graph(count-1);
    g->setPen(QPen(penColor));
    if (useBrush)
        g->setBrush(QBrush(QColor(0, 0, 255, 20)));
    g->setData(x, y);

    cp->xAxis->setRange(x.first(), x.last());

    double min = getMin(y);
    double max = getMax(y);

    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

    cp->yAxis->setNumberFormat("f");
    cp->yAxis->setNumberPrecision(2);

    cp->replot();
    return g;
}

bool PairTabPage::isTrading(Security* s)
{

//    return true;

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = currentDateTime.date();

    ContractDetails* cd = s->getContractDetails();
    QString timeZoneId = cd->timeZoneId;
    QString liquidTradingHours = cd->liquidHours.split(';').at(0).split(':').at(1);


    if (liquidTradingHours == "CLOSED")
        return false;


    QByteArray IanaTimeZoneString;

    if (timeZoneId == "EST5EDT") {
        IanaTimeZoneString = "America/New_York";
    }
    else if (timeZoneId == "CST") {
        IanaTimeZoneString = "America/Chicago";
    }

    QTimeZone tz(IanaTimeZoneString);

    QDateTime startDT;
    QDateTime endDT;


    if (liquidTradingHours.contains(',')) {
        QStringList lthList = liquidTradingHours.split(',');
        foreach(QString lth, lthList) {
// pDebug(lthList);
            QTime startTime = QTime::fromString(lth.split('-').at(0), "hhmm");
            QTime endTime   = QTime::fromString(lth.split('-').at(1), "hhmm");

            startDT.setDate(currentDate);
            startDT.setTime(startTime);
            startDT.setTimeZone(tz);
            startDT = startDT.toLocalTime();

            endDT.setDate(currentDate);
            endDT.setTime(endTime);
            endDT.setTimeZone(tz);
            endDT = endDT.toLocalTime();
        }
    }
    else {
        QString lth = liquidTradingHours;

// pDebug(lth);

        QTime startTime = QTime::fromString(lth.split('-').at(0), "hhmm");
        QTime endTime   = QTime::fromString(lth.split('-').at(1), "hhmm");

        startDT.setDate(currentDate);
        startDT.setTime(startTime);
        startDT.setTimeZone(tz);
        startDT = startDT.toLocalTime();

        endDT.setDate(currentDate);
        endDT.setTime(endTime);
        endDT.setTimeZone(tz);
        endDT = endDT.toLocalTime();
    }

//    pDebug(startDT);
//    pDebug(endDT);


//    qDebug() << "current:" << currentDateTime;
//    qDebug() << "start:" << startDT;
//    qDebug() << "end:" << endDT;

//    qApp->exit();

    if (currentDateTime > startDT && currentDateTime < endDT) {
        return true;
//        pDebug("true");
    }

//    pDebug("false");
    return false;
}

bool PairTabPage::reqDeletePlotsAndTableRow()
{
    if (!reqClosePair()) {
//        pDebug("denied");
        return false;
    }

    QMdiArea* mdi = ui->mdiArea;
    while (mdi->subWindowList().size()) {
        for (int i=0;i<mdi->subWindowList().size();++i) {
            QMdiSubWindow* sw = mdi->subWindowList().at(i);
            sw->setAttribute(Qt::WA_DeleteOnClose);
            QWidget* plot = sw->widget();
            delete plot;
            mdi->removeSubWindow(sw);
            sw->close();
            if (sw)
                delete sw;
        }
    }


//    QString dStr("subwindowList.size():" + mdi->subWindowList().size());
//    pDebug(mdi->subWindowList().size());

    m_pair1ShowButtonClickedAlready = false;
    m_pair2ShowButtonClickedAlready = false;

    return true;
}


void PairTabPage::on_timeFrameComboBox_currentIndexChanged(const QString &arg1)
{
    m_timeFrameString = arg1;
}

void PairTabPage::onCustomPlotDoubleClick(QCPAbstractPlottable* plotable, QMouseEvent* event)
{
    pDebug("");
    Q_UNUSED(event);

    QCPGraph* graph = qobject_cast<QCPGraph*>(plotable);
    double first = graph->data()->keys().first();
    double last  = graph->data()->keys().last();
    graph->keyAxis()->setRange(first, last);
    graph->parentPlot()->replot();
}

void PairTabPage::onResetPlot()
{
    pDebug("");
    QPoint point = QCursor::pos();
    QCPAbstractPlottable* plotable = NULL;
    foreach(QCustomPlot* cp, m_customPlotMap.values()) {
        plotable = cp->plottableAt(point);
        if (plotable != 0)
            break;
    }

    if (!plotable)
        return;

    QCPGraph* graph = qobject_cast<QCPGraph*>(plotable);
    double first = graph->data()->keys().first();
    double last  = graph->data()->keys().last();
    graph->keyAxis()->setRange(first, last);
    graph->parentPlot()->replot();
}



void PairTabPage::removeTableRow()
{
    QTableWidget* tab = mwui->homeTableWidget;

    for (int r=0;r<tab->rowCount();++r) {
        if (tab->item(r,0)->text() == m_oldTabSymbol) {
            tab->removeRow(r);
            tab->update();
        }
    }
}
void PairTabPage::on_manualTradeEntryCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        ui->activateButton->setText("Place Order");
    }
    else
        ui->activateButton->setText("Activate");
}

void PairTabPage::on_manualTradeExitCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        ui->deactivateButton->setText("Close Order");
    }
    else
        ui->deactivateButton->setText("Deactivate");
}

void PairTabPage::onMouseMove(QMouseEvent *event)
{
    QCustomPlot* cp = qobject_cast<QCustomPlot*>(sender());

    double xVal = 0;
    double yVal = 0;

    xVal = cp->xAxis->pixelToCoord((double)(event->pos().x()));
    yVal = cp->yAxis->pixelToCoord((double)(event->pos().y()));
    m_mainWindow->statusBar()->clearMessage();

    QDateTime dt = QDateTime::fromTime_t(xVal);

    QString dateString(QString("Date: " + dt.toString("MM/dd/yyyy")));
    QString timeString(QString("Time: " + dt.toString("hh:mm:ss")));
    QString valString(QString("Value: " + QString::number(yVal)));
    m_mainWindow->statusBar()->showMessage(dateString + "        " + timeString + "        " + valString);
}

int PairTabPage::getPairTabPageId() const
{
    return m_pairTabPageId;
}

void PairTabPage::setPairTabPageId(int pairTabPageId)
{
    m_pairTabPageId = pairTabPageId;
}


QMap<long, Security *> PairTabPage::getSecurityMap() const
{
    return m_securityMap;
}
