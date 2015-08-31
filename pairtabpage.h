#ifndef PAIRTABPAGE_H
#define PAIRTABPAGE_H

#include "ibticktype.h"
#include "security.h"
#include "iborder.h"
#include "iborderstate.h"

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QList>
#include <QPair>

class IBClient;
struct Contract;
class ContractDetailsWidget;

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

    ContractDetailsWidget *getPair1ContractDetailsWidget() const;

    ContractDetailsWidget *getPair2ContractDetailsWidget() const;

public slots:
    void onHistoricalData(long reqId, const QByteArray& date, double open, double high,
        double low, double close, int volume, int barCount, double WAP, int hasGaps);

private slots:
//    void on_pair1SymbolLineEdit_textEdited(const QString &arg1);
//    void on_pair2SymbolLineEdit_textEdited(const QString &arg1);
//    void on_pair1SymbolLineEdit_editingFinished();
//    void on_pair2SymbolLineEdit_editingFinished();
    void on_pair1ShowButton_clicked();
    void on_pair2ShowButton_clicked();
//    void on_pair1PrimaryExchangeLineEdit_textEdited(const QString &arg1);
//    void on_pair2PrimaryExchangeLineEdit_textEdited(const QString &arg1);

    void onPair1TimeOut();
    void onPair2TimeOut();

    void onActivateButtonClicked(bool);
    void onDeactivateButtonClicked(bool);

    void onOrderStatus(long orderId, const QByteArray &status, int filled,
                                        int remaining, double avgFillPrice, int permId, int parentId,
                                        double lastFillPrice, int clientId, const QByteArray& whyHeld);
    void onOpenOrder(long orderId, const Contract& contract, const Order& order, const OrderState& orderState);

    void onSingleShotTimer();
    void onContractDetails(int reqId, const ContractDetails & contractDetails);
    void onTradeEntryNumStdDevLayersChanged(int num);
    void onWaitCheckBoxStateChanged(int state);
    void onTrailCheckBoxStateChanged(int state);
    void onCdui1SymbolTextChanged(QString text);
    void onCdui2SymbolTextChanged(QString text);


private:
    IBClient*                               m_ibClient;
    Ui::PairTabPage*                        ui;
    Ui::MainWindow*                         mwui;
    QMap<long,Security*>                    m_securityMap;
    QMap<long, long>                        m_newBarMap;
    QMap<long, long>                        m_contractDetailsMap;
    TimeFrame                               m_timeFrame;
    uint                                    m_timeFrameInSeconds;
    QMap<int, QCustomPlot*>                 m_customPlotMap;
    QVector<double>                         m_ratio;
    QVector<double>                         m_ratioStdDev;
    QVector<double>                         m_ratioMA;
    QVector<double>                         m_ratioRSI;
    QVector<double>                         m_ratioPercentFromMean;
    QVector<double>                         m_correlation;
    QVector<double>                         m_cointegration;
    QVector<double>                         m_ratioVolatility;
    QVector<double>                         m_spreadRSI;
    QString                                 m_origButtonStyleSheet;

    QVector<Order*>                         m_orders;         // replace this w/ a list of openOrders

    bool                                    m_ratioRSITriggerActivated;
    bool                                    m_percentFromMeanTriggerActivated;
    bool                                    m_stdDevLayer1TriggerActivated;
    bool                                    m_stdDevLayer2TriggerActivated;
    bool                                    m_stdDevLayer3TriggerActivated;
    bool                                    m_stdDevLayer4TriggerActivated;
    bool                                    m_stdDevLayer5TriggerActivated;
    QVector<double>                         m_stdDevLayerPeaks;

    ContractDetailsWidget*                  m_pair1ContractDetailsWidget;
    ContractDetailsWidget*                  m_pair2ContractDetailsWidget;

    enum ChartType
    {
        LINE,
        CANDLE,
        OHLC,
        BAR,
        SCATTER
    };

    void reqHistoricalData(long tickerId);
    void placeOrders();
    void placeOrder();
    void exitOrder();
    void showPlot(long tickerId, ChartType chartType);
    void appendPlot(long tickerId);
    void plotRatio();
    void plotRatioMA();
    void plotRatioStdDev();
    void plotRatioPercentFromMean();
    void plotCorrelation();
    void plotCointegration();
    void plotRatioVolatility();
    void plotRatioRSI();
    void plotSpreadRSI();
    void addTableRow();
    void checkTradeTriggers();
    void checkTradeExits();
//    void setupTriggers();``
};

#endif // PAIRTABPAGE_H
