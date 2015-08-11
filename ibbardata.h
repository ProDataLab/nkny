#ifndef IBBARDATA_H
#define IBBARDATA_H

#include <QByteArray>

struct BarData {
    QByteArray date;
    double open;
    double high;
    double low;
    double close;
    int volume;
    double average;
    QByteArray hasGaps;
    int barCount;
};

#endif // IBBARDATA_H

