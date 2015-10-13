#ifndef SECURITY_H
#define SECURITY_H

#include "qcustomplot.h"
#include "ibcontract.h"
#include "iborder.h"
#include "iborderstate.h"
#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QVector>
#include <QDateTime>
#include <QTimer>

enum TimeFrame
{
    SEC_1,
    SEC_5,
    SEC_15,
    SEC_30,
    MIN_1,
    MIN_2,
    MIN_3,
    MIN_5,
    MIN_15,
    MIN_30,
    HOUR_1,
    DAY_1,
    RAW
};


struct DataVecs {};

struct DataVecsHist : public DataVecs
{
    QVector<double> timeStamp;
    QVector<double> open;
    QVector<double> high;
    QVector<double> low;
    QVector<double> close;
    QVector<uint>   volume;
    QVector<uint>   barCount;
    QVector<double> wap;
    QVector<bool>   hasGaps;
};
\
struct DataVecsNewBar : public DataVecs
{
    QVector<double> timeStamp;
    QVector<double> open;
    QVector<double> high;
    QVector<double> low;
    QVector<double> close;
    QVector<uint>   volume;
    QVector<uint>   barCount;
    QVector<double> wap;
    QVector<bool>   hasGaps;
};

struct DataVecsMoreHist : public DataVecsHist {};


struct DataVecsRaw : public DataVecs
{
    QVector<double> timeStamp;
    QVector<double> price;
    QVector<int>    size;
};

//#define DataVecsFill DataVecsHist

enum TriggerType
{
    LAYER_1=0,
    LAYER_2,
    LAYER_3,
    LAYER_4,
    LAYER_5,
    RSI,
    PCNT,
    EXIT,
    MANUAL,
    TEST
};

struct SecurityOrder
{
    Order order;
    OrderState orderState;
    QByteArray status;
    int filled;
    int remaining;
    double avgFillPrice;
    int permId;
    int parentId;
    double lastFillPrice;
    int clientId;
    QByteArray whyHeld;
    TriggerType triggerType;                   // used to distiguish various orders by layer
    long referenceOrderId;
};


class PairTabPage;



class Security : public QObject
{
    Q_OBJECT

public:
    Security(const long & tickerId, QObject* parent=0);
    ~Security();

    void setHistoricalTickerId(const long & tickerId) { m_historicalTickerId = tickerId; }
    long getHistoricalTickerId() const { return m_historicalTickerId; }
    Contract* contract() { return &(m_contractDetails.summary); }
    bool histDataRequested() const { return m_histDataRequested; }
    void setHistDataRequested() { m_histDataRequested = true; }
    void appendHistData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);

    void appendNewBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);
    void appendMoreBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);
    void appendRawPrice(const double & price);
    void appendRawSize(const int & size);

//    DataVecsRaw* getRawData() { return (DataVecsRaw*)m_dataMap[RAW]; }
    DataVecsHist* getHistData(TimeFrame timeFrame) { return (DataVecsHist*)m_dataMap.value(timeFrame); }
    DataVecsNewBar* getNewBarData(TimeFrame timeFrame) { return (DataVecsNewBar*)m_dataMap.value(timeFrame); }

    double getLastBarsTimeStamp() const { return m_lastBarsTimeStamp; }
    void setLastBarsTimeStamp(double timeStamp) { m_lastBarsTimeStamp = timeStamp; }

//    void handleFillData(TimeFrame timeFrame);
//    bool fillDataHandled() const { return m_fillDataHandled; }
//    void handleRawData(TimeFrame timeFrame);
    void handleNewBarData(TimeFrame timeFrame);
    QTimer* getTimer() { return &m_timer; }

    void fixHistDataSize(TimeFrame timeFrame, int size);

    ContractDetails* getContractDetails();
    void setContractDetails(const ContractDetails &contractDetails);

    QMap<long, SecurityOrder *>* getSecurityOrderMap();

    SecurityOrder* newSecurityOrder(long orderId);

    Security *getPairPartner() const;
    void setPairPartner(Security *pairPartner);

    long getRealTimeTickerId() const;
    void setRealTimeTickerId(long realTimeTickerId);

    void handleRawBarData();

signals:

public slots:

private:
    long                                m_historicalTickerId;
    long                                m_realTimeTickerId;
    ContractDetails                     m_contractDetails;
    QMap<TimeFrame, DataVecs*>          m_dataMap;
//    QMap<TimeFrame, DataVecsFill*>      m_dataFillMap;
    QMap<TimeFrame, DataVecsMoreHist*>  m_moreBarsDataMap;
    QMap<TimeFrame, DataVecsNewBar*>    m_newBarDataMap;
    QMap<TimeFrame, DataVecsRaw*>       m_rawDataMap;
    bool                                m_histDataRequested;
    double                              m_lastBarsTimeStamp;
//    bool                                m_gettingRealTimeData;
//    bool                                m_fillDataHandled;
    QTimer                              m_timer;
    QMap<long,SecurityOrder*>           m_securityOrderMap;
    Security*                           m_pairPartner;
    PairTabPage*                        m_pairTabPage;
};

#endif // SECURITY_H
