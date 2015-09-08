#include "security.h"
#include "helpers.h"

Security::Security(const long &tickerId, QObject *parent)
    : QObject(parent)
    , m_tickerId(tickerId)
    , m_histDataRequested(false)
    , m_lastBarsTimeStamp(0)
//    , m_gettingRealTimeData(false)
//    , m_fillDataHandled(false)
    , m_order(NULL)
{
//    qDebug() << "[DEBUG-Security] tickerId:" << tickerId;
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
        dvh = (DataVecsHist*)m_dataMap[timeFrame];

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
        dvn = m_newBarDataMap[timeFrame];

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
    DataVecsNewBar* dvn = m_newBarDataMap[timeFrame];

    qDebug() << "[DEBUG-handleNewBarData] numNewBars:" << dvn->timeStamp.size();

    DataVecsHist*   dvh =(DataVecsHist*) m_dataMap[timeFrame];

    QDateTime lbdt = QDateTime::fromTime_t(m_lastBarsTimeStamp);

    for (int i=0;i<dvn->timeStamp.size();++i) {

        QDateTime dt = QDateTime::fromTime_t(dvn->timeStamp.at(i));

        if (dt > lbdt) {

            dvh->timeStamp.append(dt.toTime_t());
            dvh->open.append(dvn->open.at(i));
            dvh->high.append(dvn->high.at(i));
            dvh->low.append(dvn->low.at(i));
            dvh->close.append(dvn->close.at(i));
            dvh->volume.append(dvn->volume.at(i));
            dvh->barCount.append(dvn->barCount.at(i));
            dvh->wap.append(dvn->wap.at(i));
            dvh->hasGaps.append(dvn->hasGaps.at(i));
        }
    }

    m_newBarDataMap.remove(timeFrame);
}

Order *Security::getOrder()
{
    if (!m_order)
        m_order = new Order();
    return m_order;
}

void Security::fixHistDataSize(TimeFrame timeFrame)
{
    DataVecsHist* dvh = (DataVecsHist*)m_dataMap[timeFrame];
    int n = dvh->timeStamp.size() - 250;

    for (int i = 0;i<n;++i) {
        dvh->timeStamp.remove(i);
        dvh->open.remove(i);
        dvh->high.remove(i);
        dvh->low.remove(i);
        dvh->close.remove(i);
        dvh->volume.remove(i);
        dvh->barCount.remove(i);
        dvh->wap.remove(i);
        dvh->hasGaps.remove(i);
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



























