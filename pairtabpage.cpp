#include "pairtabpage.h"
#include "ui_pairtabpage.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "qcustomplot.h"
#include "helpers.h"
#include "security.h"

#include <QDateTime>
#include <QSpinBox>
#include <QPair>


PairTabPage::PairTabPage(IBClient *ibClient, QWidget *parent)
    : m_ibClient(ibClient)
    , QWidget(parent)
    , ui(new Ui::PairTabPage)

{
    ui->setupUi(this);
}

PairTabPage::~PairTabPage()
{
    delete ui;

    // TODO: IS THIS CORRECT ??????

    qDeleteAll(m_securityMap);
}


void PairTabPage::on_pair1SecurityLineEdit_editingFinished()
{
    ui->pair1ShowButton->setEnabled(true);
}

void PairTabPage::on_pair2SecurityLineEdit_editingFinished()
{
    ui->pair2ShowButton->setEnabled(true);
}

void PairTabPage::on_pair1SecurityLineEdit_textEdited(const QString &arg1)
{
    ui->pair1SecurityLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair2SecurityLineEdit_textEdited(const QString &arg1)
{
    ui->pair2SecurityLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair1ShowButton_clicked()
{
//    m_pair1TickerId = m_ibClient->getTickerId();

    m_pairList.append(QPair<Security*,Security*>());
    long tickerId = m_ibClient->getTickerId();
    Security* pair1 = m_pairList.last().first = new Security(tickerId, new Contract);
    m_securityMap[tickerId] = pair1;
    Contract* contract = pair1->contract();
    contract->conId = ui->pair1ConIdLineEdit->text().toLong();
    contract->symbol = ui->pair1SecurityLineEdit->text().toLocal8Bit();
    contract->secType = ui->secTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = "SMART";
    contract->primaryExchange = ui->pair1PrimaryExchangeLineEdit->text().toLocal8Bit();
    contract->currency = "USD";

    reqMktData(pair1->tickerId(), contract);


    ui->pair2ShowButton->setEnabled(true);
}

void PairTabPage::on_pair2ShowButton_clicked()
{
    long tickerId = m_ibClient->getTickerId();
    Security* pair2 = m_pairList.last().second = new Security(tickerId, new Contract);
    m_securityMap[tickerId] = pair2;
    Contract* contract = pair2->contract();
    contract->conId = ui->pair2ConIdLineEdit->text().toLong();
    contract->symbol = ui->pair2SecurityLineEdit->text().toLocal8Bit();
    contract->secType = ui->secTypeComboBox->currentText().toLocal8Bit();
    contract->exchange = "SMART";
    contract->primaryExchange = ui->pair2PrimaryExchangeLineEdit->text().toLocal8Bit();
    contract->currency = "USD";

    reqMktData(pair2->tickerId(), contract);
}

void PairTabPage::onHistoricalData(long reqId, const QByteArray &date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps)
{
//    qDebug() << "[HIST]" << reqId << date << open << high << low << close << volume << barCount << WAP << hasGaps;

//    20150819  DAY_1

    if (date.startsWith("finished")) {
        // done collecting hist data.... todo: show the plot
        QVector<DataPoint*> data = m_securityMap[reqId]->getData(m_timeFrame);
        foreach(DataPoint* dp, data) {
            qDebug() << "[HIST-COLLECTED]" << dp->timeStamp.toString() << dp->value;
        }

        return;
    }

    if (!m_securityMap.contains(reqId)) {
        // TOODO:: THIS IS AN ERROR THAT SHOULD NOT SHOW UP... HANDLE IT
        qDebug() << "[CRITICAL] reqId is not known!";
    }

    QDateTime dt;

    if (m_timeFrame != DAY_1)
        dt = QDateTime::fromTime_t(date.toUInt());
    else
        dt = QDateTime::fromString(date, "yyyyMMdd");

    Security* security = m_securityMap[reqId];
    security->appendData(m_timeFrame, dt, close);

}



void PairTabPage::onTickGeneric(long tickerId, TickType tickType, double value)
{
    qDebug() << "[DEBUG-GenericTick]" << tickerId << tickType << value;
}

void PairTabPage::onTickPrice(long tickerId, TickType tickType, double price, int canAutoExecute)
{
    QDateTime timeStamp = QDateTime::currentDateTime();

    Security* security = m_securityMap[tickerId];

    if (!security->histDataRequested()) {
        reqHistoricalData(tickerId, timeStamp);
        security->setHistDataRequested();
    }


//    qDebug() << "[DEBUG-TickPrice]" << tickerId << tickType << price << canAutoExecute;



    switch (tickType) {
    case BID:
        break;
    case ASK:
        break;
    case LAST:
//        qDebug() << "[RT]" << QDateTime::currentDateTime().toString() << price;
        security->appendData(RAW, timeStamp, price);
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
//    qDebug() << "[DEBUG-TickSize]" << tickerId << tickType << size;
}



void PairTabPage::reqHistoricalData(long tickerId, const QDateTime & endDate)
{
    Security* security = m_securityMap[tickerId];

    TimeFrame tf = (TimeFrame)ui->timeFrameComboBox->currentIndex();
    QByteArray barSize = ui->timeFrameComboBox->currentText().toLocal8Bit();
    QByteArray durationStr;
    int ma = ui->maSpinBox->value();

    switch (tf)
    {
    case SEC_1:
        m_timeFrame = SEC_1;
        durationStr = QByteArray::number(ma + 10) + " S";
        break;
    case SEC_5:
        m_timeFrame = SEC_5;
        durationStr = QByteArray::number((ma + 10) * 5) + " S";
        break;
    case SEC_15:
        m_timeFrame = SEC_15;
        durationStr = QByteArray::number((ma + 10) * 15) + " S";
        break;
    case SEC_30:
        m_timeFrame = SEC_30;
        durationStr = QByteArray::number((ma + 10) * 30) + " S";
        break;
    case MIN_1:
        m_timeFrame = MIN_1;
        durationStr = "1 D";
        break;
    case MIN_2:
        m_timeFrame = MIN_2;
        durationStr = "1 D";
        break;
    case MIN_3:
        m_timeFrame = MIN_3;
        durationStr = "1 D";
        break;
    case MIN_5:
        m_timeFrame = MIN_5;
        durationStr = "1 D";
        break;
    case MIN_15:
        m_timeFrame = MIN_15;
        durationStr = "1 D";
        break;
    case MIN_30:
        m_timeFrame = MIN_30;
        durationStr = "2 D";
        break;
    case HOUR_1:
        m_timeFrame = HOUR_1;
        durationStr = "5 D";
        break;
    case DAY_1:
        m_timeFrame = DAY_1;
        durationStr = QByteArray::number((ma +10)) + " D";
        break;
    case DAY_1 + 1:
        // handle 1 week time frame specially
        break;
    }

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

void PairTabPage::showPlot(const long & tickerId)
{
//    qDebug() << "[DEBUG] in showPlot";

//    QCustomPlot* customPlot = ui->customPlot;
//    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));


//    if (tickerId == m_pair1TickerId) {
//        if (customPlot->graphCount() >= 1)
//            customPlot->graph(0)->clearData();
//        addPair1Graph();
//    }
//    else {
//        if (customPlot->graphCount() > 1)
//            customPlot->graph(1)->clearData();
//        addPair2Graph();
//    }

//    // configure bottom axis to show date and time instead of number:
//    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    customPlot->xAxis->setDateTimeFormat("hh:mm:ss\ndd-MM-yy");

//    // set a more compact font size for bottom and left axis tick labels:
//    customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
//    customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

////    // set a fixed tick-step to one tick per month:
////    customPlot->xAxis->setAutoTickStep(false);
////    customPlot->xAxis->setTickStep(60 * 60); // one month in seconds
////    customPlot->xAxis->setSubTickCount(5);

////    // set axis ranges to show all data:
////    customPlot->xAxis->setRange(time, now+24*3600*249);
////    customPlot->yAxis->setRange(0, 60);

//    customPlot->replot();

}

void PairTabPage::addPair1Graph()
{
    qDebug() << "DEBUG] in addPair1Graph";
    QCustomPlot* customPlot = ui->customPlot;
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));

qDebug() << "[1]";
    if (customPlot->graphCount() == 0)
        customPlot->addGraph();

    customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(0)->setPen(pen);

qDebug() << "[2]";
    if (m_pair1Time.size() > 100 && m_pair1Close.size() > 100) {
        QVector<double> time = m_pair1Time.mid(m_pair1Time.size() - 100);
        QVector<double> close = m_pair1Close.mid(m_pair1Close.size() - 100);
        customPlot->graph(0)->setData(time, close);

        // set axis ranges to show all data:
        double min = getMin(close);
        double max = getMax(close);
        qDebug() << "[DEBUG] min:" << min << "max:" << max;
        customPlot->xAxis->setRange(time.first(), time.last());
        customPlot->yAxis->setRange(getMin(close), getMax(close));
        qDebug() << "[DEBUG] leaving addPair1Graph SIZE:" << m_pair1Time.size();

    }
    else {
        customPlot->graph(0)->setData(m_pair1Time, m_pair2Close);

        // set axis ranges to show all data:
        double min = getMin(m_pair1Close);
        double max = getMax(m_pair1Close);
        qDebug() << "[DEBUG] min:" << min << "max:" << max;
        customPlot->xAxis->setRange(m_pair1Time.first(), m_pair1Time.last());
        customPlot->yAxis->setRange(getMin(m_pair1Close), getMax(m_pair1Close));

        customPlot->xAxis->setRange(m_pair1Time.first(), m_pair1Time.last());
        customPlot->yAxis->setRange(getMin(m_pair2Close), getMax(m_pair2Close));
    }
}

void PairTabPage::addPair2Graph()
{
    QCustomPlot* customPlot = ui->customPlot;
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));

    if (customPlot->graphCount() < 2)
        customPlot->addGraph();

    customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(1)->setPen(pen);

//    QVector<double> time = m_pair2Time.mid(m_pair2Time.size() - ui->pair2MaSpinBox->value());
//    QVector<double> close = m_pair2Close.mid(m_pair2Close.size() - ui->pair2MaSpinBox->value());

    if (m_pair2Time.size() > 100 && m_pair2Close.size() > 100) {
        QVector<double> time = m_pair2Time.mid(m_pair2Time.size() - 100);
        QVector<double> close = m_pair2Close.mid(m_pair2Close.size() - 100);
        customPlot->graph()->setData(time, close);
    }
    else
        customPlot->graph(1)->setData(m_pair2Time, m_pair2Close);



}


void PairTabPage::on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair1PrimaryExchangeLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair2PrimaryExchangeLineEdit->setText(arg1.toUpper());
}
