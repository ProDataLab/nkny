#include "security.h"
#include "helpers.h"
#include "pairtabpage.h"

Security::Security(const long &tickerId, QObject *parent)
    : QObject(parent)
    , m_historicalTickerId(tickerId)
    , m_histDataRequested(false)
    , m_lastBarsTimeStamp(0)
    , m_pairTabPage(qobject_cast<PairTabPage*>(parent))
//    , m_gettingRealTimeData(false)
//    , m_fillDataHandled(false)
{
//    qDebug() << "[DEBUG-Security] tickerId:" << tickerId;
    m_rawDataMap[RAW] = new DataVecsRaw;
}

Security::~Security()
{
    qDeleteAll(m_dataMap);
}

void Security::appendHistData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps)
{
//    qDebug() << "[DEBUG-appendHistData] tickerId:" << m_tickerId;

    DataVecsHist* dvh;
    if (!m_dataMap.contains(timeFrame)) {
        dvh = new DataVecsHist;
        m_dataMap[timeFrame] = dvh;
    }
    else
        dvh = (DataVecsHist*)m_dataMap.value(timeFrame);

    dvh->timeStamp += timeStamp;
    dvh->open += open;
    dvh->high += high;
    dvh->low += low;
    dvh->close += close;
    dvh->volume += (uint)volume;
    dvh->barCount += (uint)barCount;
    dvh->wap += wap;
    dvh->hasGaps += (bool)hasGaps;
}

void Security::appendNewBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps)
{
    DataVecsNewBar* dvn;
    if (!m_newBarDataMap.contains(timeFrame)) {
        dvn = new DataVecsNewBar;
        m_newBarDataMap[timeFrame] = dvn;
    }
    else
        dvn = m_newBarDataMap.value(timeFrame);

    dvn->timeStamp += timeStamp;
    dvn->open += open;
    dvn->high += high;
    dvn->low += low;
    dvn->close += close;
    dvn->volume += (uint)volume;
    dvn->barCount += (uint)barCount;
    dvn->wap += wap;
    dvn->hasGaps += (bool)hasGaps;
}

void Security::appendMoreBarData(TimeFrame timeFrame, double timeStamp, double open, double high, double low, double close, int volume, int barCount, double wap, int hasGaps)
{
    DataVecsMoreHist* dvmh;
    if (!m_moreBarsDataMap.contains(timeFrame)) {
        dvmh = new DataVecsMoreHist;
        m_moreBarsDataMap[timeFrame] = dvmh;
    }
    else
        dvmh = m_moreBarsDataMap.value(timeFrame);
    dvmh->timeStamp += timeStamp;
    dvmh->open += open;
    dvmh->high += high;
    dvmh->low += low;
    dvmh->close += close;
    dvmh->volume += (uint)volume;
    dvmh->barCount += (uint)barCount;
    dvmh->wap += wap;
    dvmh->hasGaps += (bool)hasGaps;

}

void Security::appendRawPrice(const double &price)
{
    DataVecsRaw* dvr = m_rawDataMap.value(RAW);
    dvr->price.append(price);
//    QString tsString(QDateTime::currentDateTime().toString("yyMMdd/hh:mm:ss"));

//    qDebug() << "[DEBUG" << __func__ << "] tsString" << tsString;

    dvr->timeStamp.append((double)QDateTime::currentDateTime().toTime_t());
    dvr->size.append(-1);
}

void Security::appendRawSize(const int &size)
{
    DataVecsRaw* dvr = m_rawDataMap.value(RAW);
    if (dvr->size.size() > 0) {
        if (dvr->size.last() == -1) {
            dvr->size[dvr->size.size()-1] = size;
        }
        else {
            qDebug() << "[DEBUG-appendRawSize] duplicate size data for a single price datum";
        }
    }
    else {
        qDebug() << "[ERROR-appendRawSize] size is coming in before price";
    }
}

//void Security::handleFillData(TimeFrame timeFrame)
//{
//    DataVecsHist* dvh = (DataVecsHist*)m_dataMap[timeFrame];
//    DataVecsFill* dvf = (DataVecsFill*)m_dataFillMap[timeFrame];

//    int idx = -1;

//    for (int i=0;i<dvf->timeStamp.size();++i) {
//        if (dvf->timeStamp.at(i) == dvh->timeStamp.last()) {
//            if (i > dvf->timeStamp.size()-2) {
//                qDebug() << "[ERROR] handleFillData: bar after timestamp" << dvf->timeStamp.at(i) << "will be missing";
//                return;
//            }
//            idx = i + i;
//        }
//    }

//    dvh->timeStamp.append(dvf->timeStamp.mid(idx));
//    dvh->open.append(dvf->open.mid(idx));
//    dvh->high.append(dvf->high.mid(idx));
//    dvh->low.append(dvf->low.mid(idx));
//    dvh->close.append(dvf->close.mid(idx));
//    dvh->volume.append(dvf->volume.mid(idx));
//    dvh->barCount.append(dvf->barCount.mid(idx));
//    dvh->wap.append(dvf->wap.mid(idx));
//    dvh->hasGaps.append(dvf->hasGaps.mid(idx));

//    m_dataFillMap.remove(timeFrame);

//    m_fillDataHandled = true;
//}

//void Security::handleRawData(TimeFrame timeFrame)
//{
//    int idxOfStartTick = -1;
//    int idxOfEndTick = -1;
//    double plusSecs = 0;

//    switch (timeFrame) {
//    case SEC_1:
//        plusSecs = 1;
//        break;
//    case SEC_5:
//        plusSecs = 5;
//        break;
//    case SEC_15:
//        plusSecs = 15;
//        break;
//    case SEC_30:
//        plusSecs =30;
//        break;
//    case MIN_1:
//        plusSecs = 60;
//        break;
//    case MIN_2:
//        plusSecs = 1;
//        break;
//    case MIN_3:
//        plusSecs = 60 * 3;
//        break;
//    case MIN_5:
//        plusSecs = 60 * 5;
//        break;
//    case MIN_15:
//        plusSecs = 60 * 15;
//        break;
//    case MIN_30:
//        plusSecs = 60 * 30;
//        break;
//    case HOUR_1:
//        plusSecs = 60 * 60;
//        break;
//    case DAY_1:
//        plusSecs = 60 * 60 * 24;
//        break;
//    }

//    DataVecsRaw* dvr = (DataVecsRaw*)m_dataMap[RAW];

//    double bs = m_lastBarsTimeStamp;
//    double be = bs + plusSecs;
//    int vol = 0;
//    int barCnt = 0;

//    for (int i=1;i<dvr->timeStamp.size();++i) {
//        double tsl = dvr->timeStamp.at(i-1);
//        double tsn = dvr->timeStamp.at(i);
//        if (bs > tsl && bs < tsn) {
//            idxOfStartTick = i;
//            vol += dvr->volume.at(i);
//            ++barCnt;
//        }
//        if (idxOfStartTick >= 0) {
//            vol += dvr->volume.at(i);
//            ++barCnt;
//            if (be > tsl && be < tsn)
//                idxOfEndTick = i-1;
//        }
//    }

//    DataVecsHist* dvh = (DataVecsHist*)m_dataMap[timeFrame];

//    dvh->timeStamp.append(be);
//    dvh->open.append(dvr->close.at(idxOfStartTick));
//    dvh->high.append(getMax(dvr->close.mid(idxOfStartTick, idxOfEndTick + 1)));
//    dvh->low.append(getMin(dvr->close.mid(idxOfStartTick, idxOfEndTick + 1)));
//    dvh->close.append(dvr->close.at(idxOfEndTick));
//    dvh->volume.append(vol);
//    dvh->barCount.append(barCnt);
//    dvh->wap.append(-1);
//    dvh->hasGaps.append(-1);
//}

void Security::handleNewBarData(TimeFrame timeFrame)
{
    static bool isFirstTime = true;

    DataVecsNewBar* dvn = m_newBarDataMap.value(timeFrame);

    qDebug() << "[DEBUG-handleNewBarData] numNewBars:" << dvn->timeStamp.size();

    DataVecsHist*   dvh =(DataVecsHist*) m_dataMap.value(timeFrame);

    if (isFirstTime) {
        dvh->timeStamp.removeLast();
        m_lastBarsTimeStamp = dvh->timeStamp.last();
        dvh->open.removeLast();
        dvh->high.removeLast();
        dvh->low.removeLast();
        dvh->close.removeLast();
        dvh->volume.removeLast();
        dvh->barCount.removeLast();
        dvh->wap.removeLast();
        dvh->hasGaps.removeLast();
    }

    QDateTime lbdt = QDateTime::fromTime_t(m_lastBarsTimeStamp);

    for (int i=0;i<dvn->timeStamp.size();++i) {

        QDateTime dt = QDateTime::fromTime_t(dvn->timeStamp.at(i));

        if (dt > lbdt) {

            qDebug() << "[DEBUG-Security::handleNewBarData] appending timeStamp:" << dt.toString("yyMMdd/hh:mm:ss");

            dvh->timeStamp.append(dt.toTime_t());
            dvh->open.append(dvn->open.at(i));
            dvh->high.append(dvn->high.at(i));
            dvh->low.append(dvn->low.at(i));
            dvh->close.append(dvn->close.at(i));
            dvh->volume.append(dvn->volume.at(i));
            dvh->barCount.append(dvn->barCount.at(i));
            dvh->wap.append(dvn->wap.at(i));
            dvh->hasGaps.append(dvn->hasGaps.at(i));

            m_lastBarsTimeStamp = (double)dt.toTime_t();
        }
    }

    m_newBarDataMap.remove(timeFrame);
    isFirstTime = false;

    qDebug() << "[DEBUG-handleNewBarData] leaving";
}



void Security::fixHistDataSize(TimeFrame timeFrame, int size)
{
    DataVecsHist* dvh = (DataVecsHist*)m_dataMap.value(timeFrame);

//    qDebug() << "[DEBUG-fixHistDataSize] dvh.size:" << dvh->timeStamp.size();
//    qDebug() << "[DEBUG-fixHistDataSize] size - 250:" << n;

    for (int i = 0;i<size;++i) {
//        qDebug() << "[DEBUG-fixHistDataSize] i:" << i;
        dvh->timeStamp.removeFirst();
        dvh->open.removeFirst();
        dvh->high.removeFirst();
        dvh->low.removeFirst();
        dvh->close.removeFirst();
        dvh->volume.removeFirst();
        dvh->barCount.removeFirst();
        dvh->wap.removeFirst();
        dvh->hasGaps.removeFirst();
    }

//    qDebug() << "[DEBUG-fixHistDataSize] size:" << dvh->timeStamp.size();
}

ContractDetails *Security::getContractDetails()
{
    return &m_contractDetails;
}

void Security::setContractDetails(const ContractDetails &contractDetails)
{
    m_contractDetails = contractDetails;
}
QMap<long, SecurityOrder *> *Security::getSecurityOrderMap()
{
    return &m_securityOrderMap;
}

SecurityOrder *Security::newSecurityOrder(long orderId)
{
    SecurityOrder* so = new SecurityOrder;
    so->order.orderId = orderId;
    m_securityOrderMap[orderId] = so;
    return so;
}

Security *Security::getPairPartner() const
{
    return m_pairPartner;
}

void Security::setPairPartner(Security *pairPartner)
{
    m_pairPartner = pairPartner;
}

long Security::getRealTimeTickerId() const
{
    return m_realTimeTickerId;
}

void Security::setRealTimeTickerId(long realTimeTickerId)
{
    m_realTimeTickerId = realTimeTickerId;
}

void Security::handleRawBarData()
{        
    static bool isFirstRun = true;
    DataVecsRaw* dvr = m_rawDataMap.values().at(0);
    DataVecsHist* dvh = (DataVecsHist*)m_dataMap.values().at(0);

    double firstTimeStamp;
    double newBarsTimeStamp;

    if (isFirstRun) {
        QDateTime dt(QDateTime::fromTime_t((uint)m_lastBarsTimeStamp));
        QDateTime now = QDateTime::currentDateTime();
        while (dt < now) {
            dt = now.addSecs(m_pairTabPage->getTimeFrameInSeconds());
        }

        firstTimeStamp = (double)dt.addSecs(-m_pairTabPage->getTimeFrameInSeconds()).toTime_t();
        newBarsTimeStamp = (double)dt.toTime_t();

        isFirstRun = false;
    }
    else {
        firstTimeStamp = m_lastBarsTimeStamp;
        newBarsTimeStamp = m_lastBarsTimeStamp + m_pairTabPage->getTimeFrameInSeconds();
    }

    if (dvr->timeStamp.isEmpty()) {
        dvh->close.append(dvh->close.last());
        dvh->high.append(dvh->high.last());
        dvh->low.append(dvh->low.last());
        dvh->open.append(dvh->open.last());
    }
    else {
        double timeStamp = 0;
        double open = 0;
        double high = 0;
        double low = 999999;
        double close = 0;

        double price = 0;

        for (int i=0;i<dvr->timeStamp.size();++i) {
            timeStamp = dvr->timeStamp.at(i);
//            qDebug() << "[DEBUG-" << __func__ << "] price:" << dvr->price.at(i);
            if (timeStamp < firstTimeStamp)
                continue;
            if (timeStamp < newBarsTimeStamp) {
                price = dvr->price.at(i);
                close = price;

                if (open == 0)
                    open = price;

                if (price > high)
                    high = price;

                if (price < low)
                    low = price;

                dvr->price.remove(i);
                dvr->timeStamp.remove(i);
            }
        }
        if (close != 0) {
            dvh->open.append(open);
            dvh->high.append(high);
            dvh->low.append(low);
            dvh->close.append(close);
        }
        else {
            dvh->open.append(dvh->open.last());
            dvh->high.append(dvh->high.last());
            dvh->low.append(dvh->low.last());
            dvh->close.append(dvh->close.last());
        }
    }

    qDebug() << "[DEBUG-" << __func__ << "] newBarsTimeStamp:" << QString::number((uint)newBarsTimeStamp);
    qDebug() << "[DEBUG-" << __func__ << "] new close:" << dvh->close.last();

    dvh->timeStamp.append(newBarsTimeStamp);
    m_lastBarsTimeStamp = newBarsTimeStamp;


//    dvr->price.clear();
//    dvr->timeStamp.clear();
    dvr->size.clear();
}






























