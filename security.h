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



//#define DataVecsFill DataVecsHist

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
};


class Security : public QObject
{
    Q_OBJECT

public:
    Security(const long & tickerId, QObject* parent=0);
    ~Security();

    void setTickerId(const long & tickerId) { m_tickerId = tickerId; }
    long tickerId() const { return m_tickerId; }
    Contract* contract() { return &(m_contractDetails.summary); }
    bool histDataRequested() const { return m_histDataRequested; }
    void setHistDataRequested() { m_histDataRequested = true; }
    void appendHistData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);

    void appendNewBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);
    void appendMoreBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps);

//    void appendData(TimeFrame timeFrame, double timeStamp, double close, int size)
//    {
//        DataVecsRaw* dvr;
//        if (!m_dataMap.contains(timeFrame)) {
//            dvr = new DataVecsRaw;
//            m_dataMap[timeFrame] = dvr;
//        }
//        else
//            dvr = (DataVecsRaw*)m_dataMap[timeFrame];

//        dvr->timeStamp += timeStamp;
//        dvr->close += close;
//        dvr->volume += size;
//    }

//    void appendFillData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps)
//    {
//        DataVecsFill* dvf;
//        if (!m_dataFillMap.contains(timeFrame)) {
//            dvf = new DataVecsFill;
//            m_dataMap[timeFrame] = dvf;
//        }
//        else
//            dvf = (DataVecsHist*)m_dataMap[timeFrame];

//        dvf->timeStamp += timeStamp;
//        dvf->open += open;
//        dvf->high += high;
//        dvf->low += low;
//        dvf->close += close;
//        dvf->volume += (uint)volume;
//        dvf->barCount += (uint)barCount;
//        dvf->wap += wap;
//        dvf->hasGaps += (bool)hasGaps;
//    }

//    DataVecsRaw* getRawData() { return (DataVecsRaw*)m_dataMap[RAW]; }
    DataVecsHist* getHistData(TimeFrame timeFrame) { return (DataVecsHist*)m_dataMap[timeFrame]; }
    DataVecsNewBar* getNewBarData(TimeFrame timeFrame) { return (DataVecsNewBar*)m_dataMap[timeFrame]; }

    double lastBarsTimeStamp() const { return m_lastBarsTimeStamp; }
    void setLastBarsTimeStamp(double timeStamp) { m_lastBarsTimeStamp = timeStamp; }

//    void handleFillData(TimeFrame timeFrame);
//    bool fillDataHandled() const { return m_fillDataHandled; }
//    void handleRawData(TimeFrame timeFrame);
    void handleNewBarData(TimeFrame timeFrame);
    QTimer* getTimer() { return &m_timer; }

    void fixHistDataSize(TimeFrame timeFrame);

    ContractDetails* getContractDetails();
    void setContractDetails(const ContractDetails &contractDetails);

    QMap<long, SecurityOrder *> getSecurityOrderMap() const;

    SecurityOrder* newSecurityOrder(long orderId);

    Security *getPairPartner() const;
    void setPairPartner(Security *pairPartner);

signals:

public slots:

private:
    long                                m_tickerId;
    ContractDetails                     m_contractDetails;
    QMap<TimeFrame, DataVecs*>          m_dataMap;
//    QMap<TimeFrame, DataVecsFill*>      m_dataFillMap;
    QMap<TimeFrame, DataVecsMoreHist*>  m_moreBarsDataMap;
    QMap<TimeFrame, DataVecsNewBar*>    m_newBarDataMap;
    bool                                m_histDataRequested;
    double                              m_lastBarsTimeStamp;
//    bool                                m_gettingRealTimeData;
//    bool                                m_fillDataHandled;
    QTimer                              m_timer;
    QMap<long,SecurityOrder*>           m_securityOrderMap;
    Security*                           m_pairPartner;
};

#endif // SECURITY_H
