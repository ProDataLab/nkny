#ifndef PAIRTABPAGE_H
#define PAIRTABPAGE_H

#include "ibticktype.h"
#include "security.h"
#include "iborder.h"

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QList>
#include <QPair>

class IBClient;
struct Contract;

namespace Ui {
class PairTabPage;
class MainWindow;
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
    void on_pair1SymbolLineEdit_textEdited(const QString &arg1);
    void on_pair2SymbolLineEdit_textEdited(const QString &arg1);
    void on_pair1SymbolLineEdit_editingFinished();
    void on_pair2SymbolLineEdit_editingFinished();
    void on_pair1ShowButton_clicked();
    void on_pair2ShowButton_clicked();
    void on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1);
    void on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1);

private:
    IBClient*                               m_ibClient;
    Ui::PairTabPage*                        ui;
    Ui::MainWindow*                         mwui;
    QList<QPair<Security*,Security*> >      m_pairList;
    QMap<long,Security*>                    m_securityMap;
    TimeFrame                               m_timeFrame;
    uint                                    m_timeFrameInSeconds;
    double                                  m_lastPrice;
    QMap<long,long>                         m_fillTickerIds;
    QMap<long,long>                         m_histTickerIds;


    void reqHistoricalData(long tickerId, const QDateTime &endDate);
    void reqMktData(const long & tickerId, Contract *contract);
    void activate(long tickerId, Contract* contract, Order* order);

    void showPlot(const long &tickerId);
    void fixPlot(long tickerId);
//    void appendPlot(long tickerId);
};

#endif // PAIRTABPAGE_H
