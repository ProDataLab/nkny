#include "pairtabpage.h"
#include "ui_pairtabpage.h"
#include "ui_mainwindow.h"
#include "ui_stddevlayertab.h"
#include "ui_contractdetailswidget.h"
#include "ui_globalconfigdialog.h"

#include "contractdetailswidget.h"
#include "globalconfigdialog.h"
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
#include <QMdiArea>
#include <QMdiSubWindow>


PairTabPage::PairTabPage(IBClient *ibClient, const QStringList & managedAccounts, QWidget *parent)
    : QWidget(parent)
    , m_ibClient(ibClient)
    , m_managedAccounts(managedAccounts)
    , ui(new Ui::PairTabPage)
    , m_ratioRSITriggerActivated(false)
    , m_percentFromMeanTriggerActivated(false)
    , m_homeTablePageRowIndex(0)
    , m_gettingMoreHistoricalData(false)
    , m_bothPairsUpdated(true)
    , m_canSetTabWidgetCurrentIndex(false)
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


    setDefaults();


    cdui1->symbolLineEdit->setText("MSFT");
    cdui1->primaryExchangeLineEdit->setText("NYSE");

    cdui2->symbolLineEdit->setText("AAPL");
    cdui2->primaryExchangeLineEdit->setText("NASDAQ");

    ui->pairsTabWidget->tabBar()->setTabText(0, cdui1->symbolLineEdit->text());
    ui->pairsTabWidget->tabBar()->setTabText(1, cdui2->symbolLineEdit->text());

    ui->managedAccountsComboBox->addItems(m_managedAccounts);

//    qDebug() << "[DEBUG-PairTabPage]" << ui->pair1ShowButton->styleSheet();
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
    connect(ui->overrideUnitSizeCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(onOverrideCheckBoxStateChanged(int)));

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
//        qDebug() << "[ERROR-onHistoricalData] request Id UNKNOWN";
//        qDebug() << "    secMapKeys:" << m_securityMap.keys();
//        qDebug() << "    secMapVals:" << m_securityMap.values();
//        qDebug() << "    reqId:" << reqId;
        return;
    }

    if (!isNewBarReq) {
        if (date.startsWith("finished")) {
            DataVecsHist* dvh = s->getHistData(m_timeFrame);
            if (!isMoreDataReq) {
                double lastBarsTimeStamp = s->getHistData(m_timeFrame)->timeStamp.last();
                s->setLastBarsTimeStamp(lastBarsTimeStamp);

                s->getTimer()->start(m_timeFrameInSeconds * 1000);


                qDebug() << "[DEBUG-onHistoricalData] NUM BARS RECEIVED:" << dvh->timeStamp.size();

//                if (dvh->timeStamp.size() < ui->lookbackSpinBox->value()) {
//                    // FIXME: I need more bars (1 HOUR BARS)
//                    m_moreDataMap[s->tickerId()] = m_ibClient->getTickerId();
////                    QTimer::singleShot(1000, this, SLOT(onMoreHistoricalDataNeeded()));
//                    onMoreHistoricalDataNeeded();
//                    return;
//                }
            }
            else {  // HANDLE THE LOOKBACK DATA
                m_moreDataMap.remove(s->tickerId());
                if (dvh->timeStamp.size() < ui->lookbackSpinBox->value()) {
                    m_moreDataMap[s->tickerId()] = m_ibClient->getTickerId();
                    onMoreHistoricalDataNeeded();
                    return;
                }
            }

            showPlot(sid);

            if (m_securityMap.keys().indexOf(reqId) == 1) {

                plotRatio();

//                plotRatioMA();

                plotRatioStdDev();

                plotRatioPercentFromMean();

                plotCorrelation();

//                plotCointegration();

                plotRatioVolatility();

                plotRatioRSI();

                plotRSISpread();

                addTableRow();

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

    m_securityMap.values().at(0)->setPairPartner(m_securityMap.values().at(1));
    m_securityMap.values().at(1)->setPairPartner(m_securityMap.values().at(0));

    qDebug() << "[DEBUG-on_pair2ShowButtonClicked] m_securityMap.size():" << m_securityMap.size();

    QString exp1("");
    QString exp2("");

    if (ui->pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        exp1 = " [" + ui->pair1ContractDetailsWidget->getUi()->expiryLineEdit->text() + "]";
    }
    if (ui->pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        exp2 = " [" + ui->pair2ContractDetailsWidget->getUi()->expiryLineEdit->text() + "]";
    }
    m_tabSymbol = ui->pairsTabWidget->tabText(0)
            + exp1
            + "/"
            + exp2
            + ui->pairsTabWidget->tabText(1)
            + " (" + m_timeFrameString + ")";
    mwui->tabWidget->setTabText(mwui->tabWidget->indexOf(this),
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

        QMessageBox msgBox;
        msgBox.setText("Caution!");
        msgBox.setInformativeText("Are you sure all trade entries and exits are ok?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;

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
//    m_ibClient->reqOpenOrders();
    if (m_securityMap.count() > 2) {
        for (int i=0;i<m_securityMap.count();++i) {
            if (m_securityMap.keys().at(i) == 0 && m_securityMap.values().at(i) == NULL)
                m_securityMap.remove(m_securityMap.keys().at(i));
        }
    }
    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);

    if (s1->getSecurityOrderMap().isEmpty() && s2->getSecurityOrderMap().isEmpty()) {
        ui->activateButton->setEnabled(true);
        ui->deactivateButton->setEnabled(false);
        m_stdDevLayerPeaks.clear();
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Can not deactivate this pair because orders have been placed");
        msgBox.setInformativeText("Please go to the orders page to close existing orders");
//        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//        msgBox.setDefaultButton(QMessageBox::Save);
        /*int ret =*/ msgBox.exec();
    }
}



void PairTabPage::onSingleShotTimer()
{
    qDebug() << "[DEBUG-onSingleShotTimer]";
//    m_ibClient->reqOpenOrders();
}

void PairTabPage::onContractDetails(int reqId, const ContractDetails &contractDetails)
{
//    qDebug() << "[DEBUG-onContractDetails]"
//             << reqId
//             << contractDetails.summary.symbol
//             << contractDetails.summary.localSymbol
//             << contractDetails.summary.conId
//             << contractDetails.summary.expiry;

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
    mwui->tabWidget->setTabText(mwui->tabWidget->indexOf(this),
                                text.toUpper() + "/" + ui->pairsTabWidget->tabText(1));
    ui->pair1UnitOverrideLabel->setText(text.toUpper() + QString(" Units"));
}

void PairTabPage::onCdui2SymbolTextChanged(QString text)
{
    ui->pairsTabWidget->setTabText(1, text.toUpper());
    m_tabSymbol = ui->pairsTabWidget->tabText(0) + "/" + text.toUpper() + "(" + m_timeFrameString + ")";
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

    menu.exec(point);
}

void PairTabPage::onCascadeAct()
{
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
    ui->mdiArea->cascadeSubWindows();
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


void PairTabPage::setTabSymbol(const QString &tabSymbol)
{
    m_tabSymbol = tabSymbol;
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
    QSettings s;
    s.beginGroup(m_tabSymbol);
    s.setValue("activateButtonEnabled", ui->activateButton->isEnabled());
    s.setValue("dectivateButtonEnabled", ui->deactivateButton->isEnabled());
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
    for (int i=0;i<ui->pairsTabWidget->count();++i) {
        s.setArrayIndex(i);
        s.setValue("tabText", ui->pairsTabWidget->tabText(i));
        if (i == 0)
            s.setValue("showButton1Enabled", ui->pair1ShowButton->isEnabled());
        else
            s.setValue("showButton2Enabled", ui->pair2ShowButton->isEnabled());

        Ui::ContractDetailsWidget* c = NULL;
        if (i==0)
            c = ui->pair1ContractDetailsWidget->getUi();
        else
            c = ui->pair2ContractDetailsWidget->getUi();
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
    s.setValue("overrideUnitSizeCheckBoxState", ui->overrideUnitSizeCheckBox->checkState());
    s.setValue("pair1UnitOverride", ui->pair1UnitOverrideSpinBox->value());
    s.setValue("pair2UnitOverride", ui->pair2UnitOverrideSpinBox->value());
    s.setValue("rsiUpperCheckState", ui->tradeEntryRSIUpperCheckBox->checkState());
    s.setValue("rsiUpper", ui->tradeEntryRSIUpperSpinBox->value());
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
    s.setValue("percentStopLoss", ui->tradeExitPercentStopLossSpinBox->value());
    s.setValue("percentFromMeanCheckState", ui->tradeExitPercentFromMeanCheckBox->checkState());
    s.setValue("percentFromMean", ui->tradeExitPercentFromMeanSpinBox->value());
    s.setValue("stdDevCheckBox", ui->tradeExitStdDevCheckBox->checkState());
    s.setValue("stdDev", ui->tradeExitStdDevDoubleSpinBox->value());
    s.endGroup();
}

void PairTabPage::readSettings()
{
    QString sym1, sym2;

    QSettings s;
    s.beginGroup(m_tabSymbol);
    ui->activateButton->setEnabled(s.value("activateButtonEnabled").toBool());
    ui->deactivateButton->setEnabled(s.value("deactivateButtonEnabled").toBool());
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
        if (c->expiryLineEdit->isEnabled())
            c->expiryLineEdit->setText(s.value("expiry").toString());
        bool showButtonEnabled = false;
        if (i==0) {
            showButtonEnabled = s.value("showButton1Enabled").toBool();
            if (!showButtonEnabled)
                ui->pair1ShowButton->click();
        }
        else {
            showButtonEnabled = s.value("showButton2Enabled").toBool();
            if (!showButtonEnabled && !ui->pair1ShowButton->isEnabled()) {
                ui->pair2ShowButton->click();
            }
        }
    }
    s.endArray();
    s.endGroup();

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
    ui->tradeEntryPercentFromMeanSpinBox->setValue(s.value("percentFromMean").toInt());
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

    qDebug() << "[DEBUG-PairTabPage::readSettings] num layers:" << size;

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
    ui->tradeExitPercentStopLossSpinBox->setValue(s.value("percentStopLoss").toInt());
    ui->tradeExitPercentFromMeanCheckBox->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckBoxState").toInt());
    ui->tradeExitPercentFromMeanSpinBox->setValue(s.value("percentFromMean").toInt());
    ui->tradeExitStdDevCheckBox->setCheckState((Qt::CheckState)s.value("stdDevCheckBox").toInt());
    ui->tradeExitStdDevDoubleSpinBox->setValue(s.value("stdDev").toDouble());
    s.endGroup();

//    if (ui->tabWidget->count() > 2) {
//        s.beginGroup(m_tabSymbol);
//        ui->tabWidget->setCurrentIndex(s.value("tabWidgetIndex").toInt());
//        s.endGroup();
//    }
    if (!ui->pair1ShowButton->isEnabled() && !ui->pair2ShowButton->isEnabled())
        m_canSetTabWidgetCurrentIndex = true;
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

    double ratio1, ratio2;

    if ( !ui->overrideUnitSizeCheckBox->isChecked()) {

        if (m_ratio.last() < 1) {
            ratio1 = m_ratio.last();
            ratio2 = 1 - ratio1;
        }
        else {
            ratio1 = s2->getHistData(m_timeFrame)->close.last() / s1->getHistData(m_timeFrame)->close.last();
            ratio2 = 1 - ratio1;
        }
    }
    else {
        int numPair1 = ui->pair1UnitOverrideSpinBox->value();
        int numPair2 = ui->pair2UnitOverrideSpinBox->value();

        ratio1 = numPair1 / numPair2;
        ratio2 = 1 - ratio1;

        c1 = s1->contract();
        c2 = s2->contract();
    }

    qDebug() << "[DEBUG-placeOrder] ratio1: " << ratio1 << "ratio2:" << ratio2;

    Order o1, o2;

    o1.action = "BUY";
    o1.totalQuantity = (long)ui->tradeEntryAmountSpinBox->value() * ratio1 / s1->getHistData(m_timeFrame)->close.last();
    qDebug() << "[DEBUG-placeOrder] o1.totalQuantity:" << o1.totalQuantity;

    o1.orderType = "MKT";
    o1.transmit = true;
    o1.account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    o2.action = "SELL";
    o2.totalQuantity = (long)ui->tradeEntryAmountSpinBox->value() * ratio2 / s2->getHistData(m_timeFrame)->close.last();
    qDebug() << "[DEBUG-placeOrder] o2.totalQuantity:" << o2.totalQuantity;

    o2.orderType = "MKT";
    o2.transmit = true;
    o2.account = ui->managedAccountsComboBox->currentText().toLocal8Bit();

    m_ibClient->placeOrder(orderId1, *c1, o1);
    m_ibClient->placeOrder(orderId2, *c2, o2);




}

void PairTabPage::exitOrder()
{
    Security* s1 = m_securityMap.values().at(0);
    Security* s2 = m_securityMap.values().at(1);


    for (int i=0;i<2;++i) {
        Order o1 = s1->getSecurityOrderMap().values().at(i)->order;
        Order o2 = s2->getSecurityOrderMap().values().at(i)->order;

        if (o1.action == "BUY") {
            o1.action = "SELL";
            o2.action = "BUY";
        }
        else {
            o1.action = "BUY";
            o2.action = "SELL";
        }
        for (int j=0; s1->getSecurityOrderMap().count();++j) {
            m_ibClient->placeOrder(s1->getSecurityOrderMap().keys().at(i), *s1->contract(), o1);
        }
        for (int j=0; s2->getSecurityOrderMap().count();++j) {
            m_ibClient->placeOrder(s2->getSecurityOrderMap().keys().at(j), *s2->contract(), o2);
        }
    }
}

void PairTabPage::showPlot(long tickerId)
{
    Security* s = m_securityMap[tickerId];
    DataVecsHist* dvh = s->getHistData(m_timeFrame);

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh->timeStamp, dvh->close);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle(s->contract()->symbol);
    sw->maximumSize();
    cp->show();

//    QCustomPlot* cp = new QCustomPlot();

//    int tabIdx = ui->tabWidget->addTab(cp, s->contract()->symbol);
//    ui->tabWidget->setCurrentIndex(tabIdx);
//    m_customPlotMap[tabIdx] = cp;                   // TODO: SLOT TO DELETE CUSTOMPLOTS WHEN TAB IS REMOVED

//    cp->addGraph();
//    QCPGraph* g = cp->graph(0);
//    g->setPen(QPen(Qt::blue));
//    g->setBrush(QBrush(QColor(0, 0, 255, 20)));
//    g->setData(dvh->timeStamp, dvh->close);

////    g->setScatterStyle(QCPScatterStyle::ssCross);

//    cp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
//    cp->axisRect()->setRangeDrag(Qt::Horizontal);
//    cp->axisRect()->setRangeZoom(Qt::Horizontal);

//    cp->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
//    cp->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    if (m_timeFrame == DAY_1)
//        cp->xAxis->setDateTimeFormat("MM/dd/yy");
//    else
//        cp->xAxis->setDateTimeFormat("MM/dd/yy\nhh:mm:ss");
//    cp->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

//    cp->xAxis->setRange(dvh->timeStamp.first(), dvh->timeStamp.last());

//    double min = getMin(dvh->close);
//    double max = getMax(dvh->close);

//    cp->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

//    cp->replot();
}


void PairTabPage::appendPlotsAndTable(long tickerId)
{
    qDebug() << "[DEBUG-appendPlot]";

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

    QCustomPlot* cp = m_customPlotMap[getPlotIndexFromSymbol(s)];
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
    m_ratioStdDev = getStdDevVector(m_ratio, ui->stdDevPeriodSpinBox->value());
    m_ratioPercentFromMean = getPercentFromMean(m_ratio);
    m_correlation = getCorrelation(dvh1->close, dvh2->close);
    m_ratioVolatility = getRatioVolatility(m_ratio, ui->volatilityPeriodSpinBox->value());
    m_ratioRSI = getRSI(m_ratio, ui->rsiPeriodSpinBox->value());

    QTableWidget* tw = mwui->homeTableWidget;

//    << "Ratio"
//    << "RatioMA"
//    << "StdDev"
//    << "PcntFromMA"
//    << "Corr"
//    << "Vola"
//    << "RSI"

    double ts = dvh->timeStamp.last();

    for (int i=0;i<ui->mdiArea->subWindowList().size();++i) {
        QMdiSubWindow* w = ui->mdiArea->subWindowList().at(i);
        QString tabText = w->windowTitle();
        cp = qobject_cast<QCustomPlot*>(w->widget());

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
    Security* s1 = NULL;
    Security* s2 = NULL;
    DataVecsHist* dvh1 = NULL;
    DataVecsHist* dvh2 = NULL;

    s1 = m_securityMap.values().at(0);
    s2 = m_securityMap.values().at(1);

    dvh1 = s1->getHistData(m_timeFrame);
    dvh2 = s2->getHistData(m_timeFrame);

    QCustomPlot* cp = createPlot();

    // Ratio
    m_ratio = getRatio(dvh1->close, dvh2->close);
    addGraph(cp, dvh1->timeStamp, m_ratio);

    // MA
    int period = qMin(ui->maPeriodSpinBox->value(), qMin(dvh1->timeStamp.size(), dvh2->timeStamp.size()));
    m_ratioMA = getMA(m_ratio, period);
    int diff = dvh1->timeStamp.size() - m_ratioMA.size();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioMA, Qt::red, false);


    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("Ratio");
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
    sw->setWindowTitle("StdDev");
    sw->maximumSize();
    cp->show();
}


void PairTabPage::plotRatioPercentFromMean()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    m_ratioPercentFromMean = getPercentFromMean(m_ratio);

    int diff = dvh1->timeStamp.size() - m_ratioPercentFromMean.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioPercentFromMean);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("PcntFromMA");
    sw->maximumSize();
    cp->show();
}


void PairTabPage::plotCorrelation()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);
    DataVecsHist* dvh2 = m_securityMap.values().at(1)->getHistData(m_timeFrame);

    m_correlation = getCorrelation(dvh1->close, dvh2->close);

    int diff = dvh1->timeStamp.size() - m_correlation.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_correlation);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("Corr");
    sw->maximumSize();
    cp->show();
}

void PairTabPage::plotCointegration()
{
    for (int i=0;i<250;++i) {
        m_cointegration[i] = 0;
    }
}

void PairTabPage::plotRatioVolatility()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    int period = qMin(ui->volatilityPeriodSpinBox->value(), m_ratio.size());

    m_ratioVolatility = getRatioVolatility(m_ratio, period);

    int diff = dvh1->timeStamp.size() - m_ratioVolatility.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioVolatility);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("Vola");
    sw->maximumSize();
    cp->show();
}

void PairTabPage::plotRatioRSI()
{
    DataVecsHist* dvh1 = m_securityMap.values().at(0)->getHistData(m_timeFrame);

    int period = qMin(ui->volatilityPeriodSpinBox->value(), m_ratio.size());

    m_ratioRSI = getRSI(m_ratio, period);

    int diff = dvh1->timeStamp.size() - m_ratioRSI.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_ratioRSI);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RSI");
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

    int period1 = qMin(ui->rsiSpreadSpinBox->value(), m_ratio.size());
    int period2 = qMin(ui->rsiSpreadSpinBox->value(), m_ratio.size());


    m_pair1RSI = getRSI(dvh1->close, period1);
    m_pair2RSI = getRSI(dvh2->close, period2);
    m_rSISpread = getDiff(m_pair1RSI, m_pair2RSI);

    int diff = 0;

    if (m_pair1RSI.size() < m_pair2RSI.size())
        diff = dvh1->timeStamp.size() - m_rSISpread.size();
    else
        diff = dvh2->timeStamp.size() - m_rSISpread.size();

    QCustomPlot* cp = createPlot();
    addGraph(cp, dvh1->timeStamp.mid(diff), m_rSISpread);

    QMdiArea* ma = ui->mdiArea;
    QMdiSubWindow* sw =  ma->addSubWindow(cp);
    sw->setWindowTitle("RSISpread");
    sw->maximumSize();
    cp->show();
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
//            << "RSI"
//            << "RSISpread;



    QString sym1 = m_pair1ContractDetailsWidget->getUi()->symbolLineEdit->text();
    QString sym2 = m_pair2ContractDetailsWidget->getUi()->symbolLineEdit->text();

    if (m_pair1ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        sym1 += "(" + m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text() + ")";
    }
    if (m_pair2ContractDetailsWidget->getUi()->securityTypeComboBox->currentText() == QString("FUT")) {
        sym2 += "(" + m_pair1ContractDetailsWidget->getUi()->expiryLineEdit->text() + ")";
    }

    QStringList itemList;
    itemList << sym1
             << sym2
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
             << QString::number(m_rSISpread.last());
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
    if (!m_ratioRSITriggerActivated && ui->tradeEntryRSIUpperCheckBox->checkState() == Qt::Checked) {
        // NOTE: tradeEntryRSILowerSpinBox is not needed on ratio and entries... maybe exits
        double lastRSI = m_ratioRSI.last();
        if (lastRSI > ui->tradeEntryRSIUpperSpinBox->value()) {
            placeOrder();
            m_ratioRSITriggerActivated = true;
        }
    }

    if (!m_percentFromMeanTriggerActivated && ui->tradeEntryPercentFromMeanCheckBox->checkState() == Qt::Checked) {
        double lastPofM = m_ratioPercentFromMean.last();
        if (lastPofM > ui->tradeEntryPercentFromMeanSpinBox->value()) {
            placeOrder();
            m_percentFromMeanTriggerActivated = true;
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
    ui->managedAccountsComboBox->setCurrentIndex(dui->managedAccountsComboBox_2->currentIndex());
    ui->tradeEntryAmountSpinBox->setValue(dui->tradeEntryAmountSpinBox_2->value());
    ui->tradeEntryRSILowerCheckBox->setChecked(dui->tradeEntryRSILowerCheckBox_3->isChecked());
    ui->tradeEntryRSILowerSpinBox->setValue(dui->tradeEntryRSILowerSpinBox_3->value());
    ui->tradeEntryRSIUpperCheckBox->setChecked(dui->tradeEntryRSIUpperCheckBox_2->isChecked());
    ui->tradeEntryRSIUpperSpinBox->setValue(dui->tradeEntryRSIUpperSpinBox_2->value());
    ui->tradeEntryPercentFromMeanCheckBox->setChecked(dui->tradeEntryPercentFromMeanCheckBox_2->isChecked());
    ui->tradeEntryPercentFromMeanSpinBox->setValue(dui->tradeEntryPercentFromMeanSpinBox_2->value());
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
    ui->tradeExitPercentStopLossSpinBox->setValue(dui->tradeExitPercentStopLossSpinBox_2->value());
    ui->tradeExitPercentFromMeanCheckBox->setChecked(dui->tradeExitPercentFromMeanCheckBox_2->isChecked());
    ui->tradeExitPercentFromMeanSpinBox->setValue(dui->tradeExitPercentFromMeanSpinBox_2->value());
    ui->tradeExitStdDevCheckBox->setChecked(dui->tradeExitStdDevCheckBox_2->isChecked());
    ui->tradeExitStdDevDoubleSpinBox->setValue(dui->tradeExitStdDevDoubleSpinBox_2->value());
}

QCustomPlot *PairTabPage::createPlot()
{
    QCustomPlot* cp = new QCustomPlot();
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

    cp->replot();
    return g;
}



