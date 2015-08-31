#include "pairtabpage2.h"
#include "ui_pairtabpage.h"
#include "ui_mainwindow.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "qcustomplot.h"
#include "helpers.h"
#include "security.h"
#include "mainwindow.h"

#include <QDateTime>
#include <QTime>
#include <QSpinBox>
#include <QPair>
#include <QPen>
#include <QColor>


PairTabPage::PairTabPage(IBClient *ibClient, QWidget *parent)
    : m_ibClient(ibClient)
    , QWidget(parent)
    , ui(new Ui::PairTabPage)
    , m_lastPrice(0)

{
    ui->setupUi(this);
    mwui = qobject_cast<MainWindow*>(parent)->getUi();

    QStringList secTypes;
    secTypes += "STK";
    secTypes += "OPT";
    secTypes += "FUT";
    secTypes += "IND";
    secTypes += "FOP";
    secTypes += "CASH";
    secTypes += "BAG";
    secTypes += "NEWS";

    ui->pair1SecurityTypeComboBox->addItems(secTypes);
    ui->pair2SecurityTypeComboBox->addItems(secTypes);

}

PairTabPage::~PairTabPage()
{
    delete ui;

    // TODO: IS THIS CORRECT ??????

    qDeleteAll(m_securityMap);
}


void PairTabPage::on_pair1SymbolLineEdit_editingFinished()
{
    ui->pair1ShowButton->setEnabled(true);
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                ui->pair1SymbolLineEdit->text() + "/" + ui->pair2SymbolLineEdit->text());
}

void PairTabPage::on_pair2SymbolLineEdit_editingFinished()
{
    ui->pair2ShowButton->setEnabled(true);
    mwui->tabWidget->setTabText(mwui->tabWidget->currentIndex(),
                                ui->pair1SymbolLineEdit->text() + "/" + ui->pair2SymbolLineEdit->text());
}

void PairTabPage::on_pair1SymbolLineEdit_textEdited(const QString &arg1)
{
    ui->pair1SymbolLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair2SymbolLineEdit_textEdited(const QString &arg1)
{
    ui->pair2SymbolLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair1ShowButton_clicked()
{
//    m_pair1TickerId = m_ibClient->getTickerId();

    m_pairList.append(QPair<Security*,Security*>());
    long tickerId = m_ibClient->getTickerId();
    Security* pair1 = m_pairList.last().first = new Security(tickerId, new Contract);
    m_securityMap[tickerId] = pair1;
    Contract* contract = pair1->contract();
    contract->conId = ui->pair1ContractIdLineEdit->text().toLong();
    contract->symbol = ui->pair1SymbolLineEdit->text().toLocal8Bit();
    contract->secType = ui->secTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = "SMART";
    contract->primaryExchange = ui->pair1PrimaryExchLineEdit->text().toLocal8Bit();
    contract->currency = "USD";


//    reqMktData(pair1->tickerId(), contract);
    reqHistoricalData2(tickerId);


    ui->pair2ShowButton->setEnabled(true);
}

void PairTabPage::on_pair2ShowButton_clicked()
{
    long tickerId = m_ibClient->getTickerId();
    Security* pair2 = m_pairList.last().second = new Security(tickerId, new Contract);
    m_securityMap[tickerId] = pair2;
    Contract* contract = pair2->contract();
    contract->conId = ui->pair2ContractIdLineEdit->text().toLong();
    contract->symbol = ui->pair2SymbolLineEdit->text().toLocal8Bit();
    contract->secType = ui->secTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = "SMART";
    contract->primaryExchange = ui->pair2PrimaryExchLineEdit->text().toLocal8Bit();
    contract->currency = "USD";

    reqMktData(pair2->tickerId(), contract);
}

void PairTabPage::onHistoricalData(long reqId, const QByteArray &date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps)
{    
    double timeStamp;

    Security* security = NULL;

    if (m_histTickerIds.values().contains(reqId)) {         // IS HIST DATA
        int hidx = m_histTickerIds.values().indexOf(reqId);
        long id = m_histTickerIds.keys().at(hidx);
        security = m_securityMap[id];

        if (date.startsWith("finished")) {
            double lastBarsTimeStamp = security->getHistData(m_timeFrame)->timeStamp.last();
//            qDebug() << "[DEBUG-onHistoricalData] lastBarsTimeStamp:" << QDateTime::fromTime_t(lastBarsTimeStamp);
            qDebug() << "[DEBUG-onHistoricalData] NUM hist bars collected:" << security->getHistData(m_timeFrame)->timeStamp.size();
            security->setLastBarsTimeStamp(lastBarsTimeStamp);

            // done collecting hist data.... todo: show the plot
            showPlot(id);

            return;
        }

        if (m_timeFrame == DAY_1)
            timeStamp = (double)QDateTime::fromString(date, "yyyyMMdd").toTime_t();
        else
            timeStamp = date.toDouble();

//        qDebug() << "[DEBUG-onHistoricalData-A]" << reqId << QDateTime::fromTime_t(timeStamp) << open << high << low << close << volume << barCount << WAP << hasGaps;

        security->appendHistData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
    }

    else if (m_fillTickerIds.values().contains(reqId)) {     // IS FILL DATA
        int fidx = m_fillTickerIds.values().indexOf(reqId);
        long id = m_fillTickerIds.keys().at(fidx);
        security = m_securityMap[id];

        if (date.startsWith("finished")) {
            security->handleFillData(m_timeFrame);
            fixPlot(id);
            return;
        }

        if (m_timeFrame == DAY_1)
            timeStamp = (double)QDateTime::fromString(date, "yyyyMMdd").toTime_t();
        else
            timeStamp = date.toDouble();

//        qDebug() << "[DEBUG-onHistoricalData-B]" << reqId << QDateTime::fromTime_t(timeStamp) << open << high << low << close << volume << barCount << WAP << hasGaps;


        security->appendFillData(m_timeFrame, timeStamp, open, high, low, close, volume, barCount, WAP, hasGaps);
    }
    else        // THIS IS AN ERROR.. HANDLE IT
        qCritical() << "TickerId in onHistoricalData.. NOT KNOWN!!";
}



void PairTabPage::onTickGeneric(long tickerId, TickType tickType, double value)
{
    qDebug() << "[DEBUG-GenericTick]" << tickerId << tickType << value;
}

void PairTabPage::onTickPrice(long tickerId, TickType tickType, double price, int canAutoExecute)
{


//    qDebug() << "[DEBUG-TickPrice]" << tickerId << tickType << price << canAutoExecute;



    switch (tickType) {
    case BID:
        break;
    case ASK:
        break;
    case LAST:
//        qDebug() << "[RT]" << QDateTime::currentDateTime().toString() << price;
//        security->appendData(RAW, (double)timeStamp.toTime_t(), price);
        m_lastPrice = price;
        break;
    case HIGH:
        break;
    case LOW:
        break;
    case CLOSE:
        break;
    }
}

void PairTabPage::onTickSize(long tickerId, TickType tickType, int size)
{
    if (tickType != LAST_SIZE)
        return;

    QDateTime timeStamp = QDateTime::currentDateTime();

//    QDate friday(2015,8,21);
//    QTime time(15, 30);
//    QDateTime timeStamp = QDateTime(friday,time,Qt::LocalTime);

//    qDebug() << "[DEBUG-onTickSize]" << tickerId << timeStamp << tickType << size;


    Security* security = m_securityMap[tickerId];

    security->appendData(RAW, (double)timeStamp.toTime_t(), m_lastPrice, size);


    if (!security->histDataRequested()) {
        security->setHistDataRequested();
        long histTickerId = m_ibClient->getTickerId();
        m_histTickerIds[tickerId] = histTickerId;
        reqHistoricalData(histTickerId, timeStamp);
        return;
    }

    double lbtTimeT = security->lastBarsTimeStamp();

    if (!lbtTimeT)          // prevents general onTickSize from interfering with rt bar binning
        return;

    QDateTime lastBarsTimeStamp = QDateTime::fromTime_t((uint)lbtTimeT);

    bool getFillData = false;

    switch (ui->timeFrameComboBox->currentIndex())
    {
    case SEC_1:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 1)
            getFillData = true;
        break;
    case SEC_5:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 5)
            getFillData = true;
        break;
    case SEC_15:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 15)
            getFillData = true;
        break;
    case SEC_30:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 30)
            getFillData = true;
        break;
    case MIN_1:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60)
            getFillData = true;
        break;
    case MIN_2:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 2)
            getFillData = true;
        break;
    case MIN_3:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 3)
            getFillData = true;
        break;
    case MIN_5:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 5)
            getFillData = true;
        break;
    case MIN_15:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 15)
            getFillData = true;
        break;
    case MIN_30:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 30)
            getFillData = true;
        break;
    case HOUR_1:
        if (lastBarsTimeStamp.secsTo(timeStamp) > 60 * 60)
            getFillData = true;
        break;
    case DAY_1:
        if (lastBarsTimeStamp.daysTo(timeStamp) > 1)
            getFillData = true;
        break;
    }

    if (getFillData) {
        qDebug() << "[DEBUG-onTickSize-02] getFillData == true .. \n\t\tlastBarsTimeStamp:"
                 << lastBarsTimeStamp
                 << "\n\t\tcurrentTimeStamp:"
                 << timeStamp;
        if (!security->fillDataHandled()) {
            long fillTickerId = m_ibClient->getTickerId();
            m_fillTickerIds[tickerId] = fillTickerId;
            reqHistoricalData(fillTickerId, lastBarsTimeStamp);
        }
        else {
            security->handleRawData(m_timeFrame);
            fixPlot(tickerId);
        }
    }
}



void PairTabPage::reqHistoricalData(long tickerId, const QDateTime & endDate)
{
    qDebug() << "[DEBUG-reqHistoricalData]" << tickerId << endDate;

    bool isFillReq = false;
    Security* security = NULL;

    for (int i=0;i<m_fillTickerIds.values().size();++i) {
        if (tickerId == m_fillTickerIds.values().at(i)) {
            isFillReq = true;
            security = m_securityMap[m_fillTickerIds.keys().at(i)];
        }
    }

    if (!isFillReq) {
        for (int i = 0;i< m_histTickerIds.values().size(); ++i) {
            if (tickerId == m_histTickerIds.values().at(i)) {
                security = m_securityMap[m_histTickerIds.keys().at(i)];
            }
            else
                qDebug() << "[ERROR] WHAT WHAT WHAT";
        }
    }

    if (!security)
        qDebug() << "[ERROR] .. i should have a security obj by now";


    TimeFrame tf = (TimeFrame)ui->timeFrameComboBox->currentIndex();
    QByteArray barSize = ui->timeFrameComboBox->currentText().toLocal8Bit();
    QByteArray durationStr;

    /*
     *  390 mins in a trading day
     */

    switch (tf)
    {
    case SEC_1:
        m_timeFrame = SEC_1;
        m_timeFrameInSeconds = 1;
        if (!isFillReq)
            durationStr = "250 S";
        else
            durationStr = "10 S";
        break;
    case SEC_5:
        m_timeFrame = SEC_5;
        m_timeFrameInSeconds = 5;
        if (!isFillReq)
            durationStr = QByteArray::number((250) * 5) + " S";
        else
            durationStr = "15 S";
        break;
    case SEC_15:
        m_timeFrame = SEC_15;
        m_timeFrameInSeconds = 15;
        if (!isFillReq)
            durationStr = QByteArray::number((250) * 15) + " S";
        else
            durationStr = "25 S";
        break;
    case SEC_30:
        m_timeFrame = SEC_30;
        m_timeFrameInSeconds = 30;
        if (!isFillReq)
            durationStr = QByteArray::number((250) * 30) + " S";
        else
            durationStr = "50 S";
        break;
    case MIN_1:
        m_timeFrame = MIN_1;
        m_timeFrameInSeconds = 60;
        if (!isFillReq)
            durationStr = "1 D";
        else
            durationStr = "1 D";
        break;
    case MIN_2:
        m_timeFrame = MIN_2;
        m_timeFrameInSeconds = 120;
        if (!isFillReq)
            durationStr = "2 D";
        else
            durationStr = "1 D";
        break;
    case MIN_3:
        m_timeFrame = MIN_3;
        m_timeFrameInSeconds = 180;
        if (!isFillReq)
            durationStr = "2 D";
        else
            durationStr = "1 D";
        break;
    case MIN_5:
        m_timeFrame = MIN_5;
        m_timeFrameInSeconds = 60 * 5;
        if (!isFillReq)
            durationStr = "4 D";
        else
            durationStr = "1 D";
        break;
    case MIN_15:
        m_timeFrame = MIN_15;
        m_timeFrameInSeconds = 60 * 15;
        if (!isFillReq)
            durationStr = "10 D";
        else
            durationStr = "1 D";
        break;
    case MIN_30:
        m_timeFrame = MIN_30;
        m_timeFrameInSeconds = 60 * 30;
        if (!isFillReq)
            durationStr = "20 D";
        else
            durationStr = "1 D";
        break;
    case HOUR_1:
        m_timeFrame = HOUR_1;
        m_timeFrameInSeconds = 60 * 60;
        if (!isFillReq)
            durationStr = "4 W";            // FIXME: I can only get 140 data points for 1 hour.. need to call again ?
        else
            durationStr = "1 D";
        break;
    case DAY_1:
        m_timeFrame = DAY_1;
        m_timeFrameInSeconds = 60 * 60 * 24;
        if (!isFillReq)
//            durationStr = "40 W";
            durationStr = "1 Y";
        else
            durationStr = "5 D";
        break;
    case DAY_1 + 1:
        // handle 1 week time frame specially
        break;
    }

    qDebug() << "[DEBUG-reqHistoricalData-02]"
             << endDate.toUTC().toString("yyyyMMdd hh:mm:ss 'GMT'").toLocal8Bit()
             << durationStr
             << barSize;


    m_ibClient->reqHistoricalData(tickerId
                                  , *(security->contract())
                                  , endDate.toUTC().toString("yyyyMMdd hh:mm:ss 'GMT'").toLocal8Bit()
                                  , durationStr
                                  , barSize
                                  , "TRADES"
                                  , 1
                                  , 2
                                  , QList<TagValue*>());
}




void PairTabPage::reqMktData(const long &tickerId, Contract* contract)
{
    m_ibClient->reqMktData(tickerId, *(contract), "", false, QList<TagValue*>());
}

void PairTabPage::activate(long tickerId, Contract *contract, Order *order)
{
    qDebug() << "[DEBUG-activate]:" << m_pairList;

    Security* s1 = m_pairList.last().first;
    Security* s2 = m_pairList.last().second;

    Contract* c1 = s1->getContract();
    Contract* c2 = s2->getContract();

    Order* o1 = s1->getOrder();
    Order* o2 = s2->getOrder();

}

void PairTabPage::showPlot(const long & tickerId)
{

//    qDebug() << "[DEBUG-showPlot]";


    QCustomPlot* customPlot = ui->customPlot;

    DataVecsHist* dvh = m_securityMap[tickerId]->getHistData((TimeFrame)m_timeFrame);

    QCPGraph* graph = new QCPGraph(customPlot->xAxis, customPlot->yAxis);

    customPlot->addPlottable(graph);

    graph->setData(dvh->timeStamp, dvh->close);


//    // create ohlc chart:
//    QCPFinancial *ohlc = new QCPFinancial(customPlot->xAxis, customPlot->yAxis);
//    customPlot->addPlottable(ohlc);
////    QCPFinancialDataMap data2 = QCPFinancial::timeSeriesToOhlc(time, value2, binSize/3.0, startTime); // divide binSize by 3 just to make the ohlc bars a bit denser
//    ohlc->setName("OHLC");
//    ohlc->setChartStyle(QCPFinancial::csOhlc);
////    ohlc->setData(&data2, true);
////    for (int i=0;i<dvh->timeStamp.size();++i) {
////        qDebug() << "[DVH]" << dvh->timeStamp[i] << dvh->open[i] << dvh->high[i] << dvh->low[i] << dvh->close[i];
////    }

//    qDebug() << "[DEBUG-showPlot] NUM BARS:" << dvh->timeStamp.size();

//    ohlc->setData(dvh->timeStamp, dvh->open, dvh->high, dvh->low, dvh->close);

//    ohlc->setWidth(m_timeFrameInSeconds * 0.8);
//    ohlc->setTwoColored(true);

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    if (m_timeFrame == DAY_1)
        customPlot->xAxis->setDateTimeFormat("MM-dd-yy");
    else
        customPlot->xAxis->setDateTimeFormat("MM-dd-yy\nhh:mm:ss");

    customPlot->xAxis->setRange(dvh->timeStamp.first(), dvh->timeStamp.last());
//    customPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    double min = getMin(dvh->close);
    double max = getMax(dvh->close);

    customPlot->yAxis->setRange(min - (min * 0.01), max + (max * 0.01));

//    customPlot->yAxis->setRange(getMin(dvh->low), getMax(dvh->high));

    // interactions
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);

    customPlot->replot();

}

void PairTabPage::fixPlot(TickerId tickerId)
{
    qDebug() << "[DEBUG-fixPlot]";

    QCPFinancial* chart = (QCPFinancial*)ui->customPlot->plottable(0);
    Security* s = m_securityMap[tickerId];
    DataVecsHist* dvh = s->getHistData(m_timeFrame);
    int i = dvh->timeStamp.indexOf(s->lastBarsTimeStamp());
    chart->addData(dvh->timeStamp.mid(i), dvh->open.mid(i), dvh->high.mid(i), dvh->low.mid(i), dvh->close.mid(i));
    s->setLastBarsTimeStamp(dvh->timeStamp.last());
    ui->customPlot->replot();
}

//void PairTabPage::appendPlot(long tickerId)
//{
//    Security* s = m_securityMap[tickerId];
//    QCPFinancial* plot = ui->customPlot->plottable(0);

//}


void PairTabPage::on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair1PrimaryExchLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair2PrimaryExchLineEdit->setText(arg1.toUpper());
}
