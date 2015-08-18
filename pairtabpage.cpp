#include "pairtabpage.h"
#include "ui_pairtabpage.h"
#include "ibclient.h"
#include "ibcontract.h"
#include "qcustomplot.h"

#include <QDateTime>
#include <QSpinBox>


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
    m_pair1TickerId = m_ibClient->getTickerId();

    reqHistoricalData(m_pair1TickerId, ui->pair1ConIdLineEdit->text().toLong()
                      , ui->pair1SecurityLineEdit->text().toLocal8Bit()
                      , ui->secTypeComboBox->currentText().toLocal8Bit()
                      , ui->pair1PrimaryExchangeLineEdit->text().toLocal8Bit()
                      , ui->timeFrameComboBox->currentIndex());

    ui->pair2ShowButton->setEnabled(true);
}

void PairTabPage::on_pair2ShowButton_clicked()
{
    m_pair2TickerId = m_ibClient->getTickerId();

    reqHistoricalData(m_pair2TickerId, ui->pair2ConIdLineEdit->text().toLong()
                      , ui->pair2SecurityLineEdit->text().toLocal8Bit()
                      , ui->secTypeComboBox->currentText().toLocal8Bit()
                      , ui->pair2PrimaryExchangeLineEdit->text().toLocal8Bit()
                      , ui->timeFrameComboBox->currentIndex());
}

void PairTabPage::onHistoricalData(long reqId, const QByteArray &date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps)
{
//    qDebug() << reqId << date << open << high << low << close << volume << barCount << WAP << hasGaps;

    if (reqId == m_pair1TickerId) {
        if (QString(date).startsWith("finish")) {
            showPlot(m_pair1TickerId);
            return;
        }
        m_pair1Time.append(date.toDouble());
        m_pair1Open.append(open);
        m_pair1High.append(high);
        m_pair1Low.append(low);
        m_pair1Close.append(close);
        m_pair1Volume.append(volume);
        m_pair1BarCount.append(barCount);
        m_pair1HasGaps.append(hasGaps);
    }
    else if (reqId == m_pair2TickerId) {
        if (QString(date).startsWith("finish")) {
            showPlot(m_pair2TickerId);
            return;
        }
        m_pair2Time.append(date.toDouble());
        m_pair2Open.append(open);
        m_pair2High.append(high);
        m_pair2Low.append(low);
        m_pair2Close.append(close);
        m_pair2Volume.append(volume);
        m_pair2BarCount.append(barCount);
        m_pair2HasGaps.append(hasGaps);
    }
}

void PairTabPage::reqHistoricalData(const long & tickerId, const long & conId, const QByteArray & symbol, const QByteArray & secType, const QByteArray & primaryExchange, const int & timeFrameIdx)
{
    Contract contract;
    contract.conId = conId;
    contract.symbol = symbol;
    contract.secType = secType;
    contract.exchange = "SMART";
    contract.primaryExchange = primaryExchange;
    contract.currency = "USD";

//    m_pair2.first = contract.symbol;
//    m_pair2.second = contract.conId;

    QDateTime dt(QDateTime::currentDateTime().toUTC());
    QByteArray dts(dt.toString("yyyyMMdd HH:mm:ss G'M'T").toLocal8Bit());

    int idx = timeFrameIdx;

    qDebug() << "[CURRENT IDX]" << idx;

    /*
    1 sec
    5 secs
    15 secs
    30 secs
    1 min
    2 mins
    3 mins
    5 mins
    15 mins
    30 mins
    1 hour
    1 day
    */

    int mult = 1;
    QByteArray duration;
    QByteArray tf;
    int dateFormat = 2;

    qDebug() << "[DEBUG] idx:" << idx;

    switch (idx)
    {
    case 0:                     // 1 secs
        mult = 1;
        duration.append("D");
        break;
    case 1:                     // 5 secs
        mult = 1;
        duration.append("D");
        break;
    case 2:                     // 10 secs
        mult = 1;
        duration.append("D");
        break;
    case 3:                     // 15 secs
        mult = 1;
        duration.append("D");
        break;\
    case 4:                     // 30 secs
        mult = 1;
        duration.append("D");
        break;
    case 5:                     // 1 min
        mult = 1;
        duration.append("D");
        break;
    case 6:                     // 2 mins
        mult = 1;
        duration.append("D");
        break;
    case 7:                     // 3 mins
        mult = 1;
        duration.append("D");
        break;
    case 8:                     // 5 mins
        mult = 1;
        duration.append("D");
        break;
    case 9:                     // 10 mins
        mult = 1;
        duration.append("W");
    case 10:                     // 15 mins
        mult = 1;
        duration.append("W");
        break;
    case 11:                     // 20 mins
        mult = 1;
        duration.append("M");
    case 12:                     // 30 mins
        mult = 1;
        duration.append("M");
        break;
    case 13:                    // 1 hour
        mult = 1;
        duration.append("W");
        break;
    case 14:                    // 2 hours
        mult = 1;
        duration.append("M");
        break;
    case 15:                    // 3 hours
        mult = 1;
        duration.append("M");
        break;
    case 16:                    // 4 hours
        mult = 1;
        duration.append("M");
        break;
    case 17:                    // 8 hours
        mult = 1;
        duration.append("M");
        break;
    case 18:                    // 1 day
        mult = 1;
        duration.append("Y");
        dateFormat = 1;
        break;
    case 19:                    // 1W
        mult = 1;
        duration.append("Y");
        dateFormat = 1;
        break;
    case 20:                    // 1M
        mult = 1;
        dateFormat = 1;
        duration.append("Y");
    }

    // TODO: THIS IS TEMPORARY !!!!!!!!!!!!!!!!!!1

    tf.append(QByteArray::number(mult));
    tf.append(" ").append(duration);

    qDebug() << "[DURATION]" << tf;

    QByteArray bs(ui->timeFrameComboBox->currentText().toLocal8Bit());

    qDebug() << "[BARSIZE]" << bs;

    m_ibClient->reqHistoricalData(tickerId
                                  , contract
                                  , dts
                                  , tf
                                  , ui->timeFrameComboBox->currentText().toLocal8Bit()
                                  , "TRADES"
                                  , 1
                                  , dateFormat
                                  , QList<TagValue*>());
}

void PairTabPage::showPlot(const long & tickerId)
{

    QCustomPlot* customPlot = ui->customPlot;
    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    customPlot->addGraph();



    if (tickerId == m_pair1TickerId)
        addPair1Graph();
    else
        addPair2Graph();


    // configure bottom axis to show date and time instead of number:
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss\ndd-MM-YY");

    // set a more compact font size for bottom and left axis tick labels:
    customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

    // set a fixed tick-step to one tick per month:
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(60 * 60); // one month in seconds
    customPlot->xAxis->setSubTickCount(5);

//    // set axis ranges to show all data:
//    customPlot->xAxis->setRange(time, now+24*3600*249);
//    customPlot->yAxis->setRange(0, 60);

    customPlot->replot();

}

void PairTabPage::addPair1Graph()
{
    QCustomPlot* customPlot = ui->customPlot;
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);

    QVector<double> time = m_pair1Time.mid(m_pair1Time.size() - ui->pair1MaSpinBox->value());
    QVector<double> close = m_pair1Close.mid(m_pair1Close.size() - ui->pair1MaSpinBox->value());
    customPlot->graph()->setData(time, close);

    // set axis ranges to show all data:
    customPlot->xAxis->setRange(time.first(), time.last());
    customPlot->yAxis->setRange(close.first(), close.last());

}

void PairTabPage::addPair2Graph()
{
    QCustomPlot* customPlot = ui->customPlot;
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);

    QVector<double> time = m_pair2Time.mid(m_pair2Time.size() - ui->pair2MaSpinBox->value());
    QVector<double> close = m_pair2Close.mid(m_pair2Close.size() - ui->pair2MaSpinBox->value());
    customPlot->graph()->setData(time, close);



}


void PairTabPage::on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair1PrimaryExchangeLineEdit->setText(arg1.toUpper());
}

void PairTabPage::on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->pair2PrimaryExchangeLineEdit->setText(arg1.toUpper());
}
