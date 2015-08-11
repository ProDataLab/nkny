#ifndef IBCOMMISSIONREPORT_H
#define IBCOMMISSIONREPORT_H

#include <QByteArray>

struct CommissionReport
{
    CommissionReport()
    {
        commission = 0;
        realizedPNL = 0;
        yield = 0;
        yieldRedemptionDate = 0;
    }

    // commission report fields
    QByteArray	execId;
    double		commission;
    QByteArray	currency;
    double		realizedPNL;
    double		yield;
    int			yieldRedemptionDate; // YYYYMMDD format
};

#endif // IBCOMMISSIONREPORT_H

