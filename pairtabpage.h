#ifndef PAIRTABPAGE_H
#define PAIRTABPAGE_H

#include "ibticktype.h"
#include "security.h"

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QList>
#include <QPair>

class IBClient;
struct Contract;

namespace Ui {
class PairTabPage;
}

class PairTabPage : public QWidget
{
    Q_OBJECT

public:
    explicit PairTabPage(IBClient* ibClient, QWidget *parent = 0);
    ~PairTabPage();
    Ui::PairTabPage* getUi() { return ui; }

public slots:
    void onHistoricalData(long reqId, const QByteArray& date, double open, double high,
        double low, double close, int volume, int barCount, double WAP, int hasGaps);

    void onTickGeneric(long tickerId, TickType tickType, double value);
    void onTickPrice(long tickerId, TickType tickType, double price, int canAutoExecute);
    void onTickSize(long tickerId, TickType tickType, int size);

private slots:
    void on_pair1SecurityLineEdit_textEdited(const QString &arg1);
    void on_pair2SecurityLineEdit_textEdited(const QString &arg1);
    void on_pair1SecurityLineEdit_editingFinished();
    void on_pair2SecurityLineEdit_editingFinished();
    void on_pair1ShowButton_clicked();
    void on_pair2ShowButton_clicked();
    void on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1);
    void on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1);

private:
    IBClient*                               m_ibClient;
    Ui::PairTabPage*                        ui;
    QList<QPair<Security*,Security*> >      m_pairList;
    QMap<long,Security*>                    m_securityMap;
    TimeFrame                               m_timeFrame;

    QVector<double> m_pair1Time;
    QVector<double> m_pair1Open;
    QVector<double> m_pair1High;
    QVector<double> m_pair1Low;
    QVector<double> m_pair1Close;
    QVector<double> m_pair1Volume;
    QVector<double> m_pair1BarCount;
    QVector<int>    m_pair1HasGaps;

    QVector<double> m_pair2Time;
    QVector<double> m_pair2Open;
    QVector<double> m_pair2High;
    QVector<double> m_pair2Low;
    QVector<double> m_pair2Close;
    QVector<double> m_pair2Volume;
    QVector<double> m_pair2BarCount;
    QVector<int>    m_pair2HasGaps;

    void reqHistoricalData(long tickerId, const QDateTime &endDate);
    void reqMktData(const long & tickerId, Contract *contract);

    void showPlot(const long &tickerId);
    void addPair1Graph();
    void addPair2Graph();
};

#endif // PAIRTABPAGE_H
