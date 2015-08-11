#ifndef IBORDERSTATE_H
#define IBORDERSTATE_H

#include "iborder.h"

struct OrderState {

    OrderState()
        :
        commission(UNSET_DOUBLE),
        minCommission(UNSET_DOUBLE),
        maxCommission(UNSET_DOUBLE)
    {}

    QByteArray status;

    QByteArray initMargin;
    QByteArray maintMargin;
    QByteArray equityWithLoan;

    double  commission;
    double  minCommission;
    double  maxCommission;
    QByteArray commissionCurrency;

    QByteArray warningText;
};

#endif // IBORDERSTATE_H

