#ifndef IBSCANDATA_H
#define IBSCANDATA_H

#include "ibcontract.h"
#include <QByteArray>

struct ScanData {
    ContractDetails contract;
    int rank;
    QByteArray distance;
    QByteArray benchmark;
    QByteArray projection;
    QByteArray legsStr;
};

#endif // IBSCANDATA_H

