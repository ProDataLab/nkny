#ifndef SECURITY_H
#define SECURITY_H

#include "qcustomplot.h"
#include "ibcontract.h"
#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QVector>
#include <QDateTime>

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


struct DataPoint
{
    QDateTime   timeStamp;
    double      value;

    DataPoint(QDateTime ts, double v) { timeStamp = ts, value = v; }
};

class Security : public QObject
{
    Q_OBJECT

public:
    Security(const long & tickerId, Contract* contract, QObject* parent=0);
    ~Security();

    void setTickerId(const long & tickerId) { m_tickerId = tickerId; }
    void setContract(Contract* contract) { m_contract = contract; }
    long tickerId() const { return m_tickerId; }
    Contract* contract() const { return m_contract; }
    bool histDataRequested() const { return m_histDataRequested; }
    void setHistDataRequested() { m_histDataRequested = true; }
    void appendData(TimeFrame timeFrame, const QDateTime & timeStamp, double value)
    {
        m_dataMap[timeFrame].append(new DataPoint(timeStamp, value));
        if (m_dataMap[RAW].size() == 10) {
            foreach(DataPoint* dp, m_dataMap[RAW]) {
                qDebug() << "[RAW-COLLECTED]" << dp->timeStamp << dp->value;
            }
        }
    }
    QVector<DataPoint*> getData(TimeFrame timeFrame) const { return m_dataMap[timeFrame]; }

signals:

public slots:

private:
    Contract*                           m_contract;
    long                                m_tickerId;
    QMap<TimeFrame, QVector<DataPoint*> >   m_dataMap;
    bool                                m_histDataRequested;
};

#endif // SECURITY_H
