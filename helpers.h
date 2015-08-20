#ifndef HELPERS_H
#define HELPERS_H

#include <QVector>
#include <QtMath>

double getMin(const QVector<double> vec)
{
    double min = 1000000;

    for (int i = 0;i < vec.size(); ++i) {
        if (vec[i] < min)
            min = vec[i];
    }
    return min;
}

double getMax(const QVector<double> vec)
{
    double max = 0;

    for (int i = 0;i < vec.size(); ++i) {
        if (vec[i] > max)
            max = vec[i];
    }
    return max;
}

double getMean(const QVector<double> & vec)
{
    double sum = 0;
    for (int i = 0; i < vec.size(); ++i) {
        sum += vec.at(i);
    }
    return sum / vec.size();
}

double getVariance(const QVector<double> & vec)
{
    double mean = getMean(vec);
    QVector<double> sqdiffVec;

    for (int i = 0; i < vec.size(); ++i) {
        double val = vec.at(i);
        sqdiffVec.append((val - mean) * (val - mean));
    }
    return getMean(sqdiffVec);
}

double getStdDev(const QVector<double> vec)
{
    return qSqrt(getVariance(vec));
}


QVector<double> getRSI(const QVector<double> vec, int period=14)
{
    int len = vec.size();

    QVector<double> deltas;
    QVector<double> seed;
    QVector<double> rsi;

    double up;
    double down;
    double rs;

    for (int i=1;i<len;++i) {
        deltas += vec.at(i) - vec.at(i-1);
    }

    seed = deltas.mid(0, period + 1);

    for (int i=0;i<seed.size();++i) {
        double val = seed.at(i);
        if (val >= 0)
            up += val;
        else
            down += val * -1;
    }

    up = up / period;
    down = down / period;

    rs = up / down;

    for (int i=0;i < period;++i) {
        rsi[i] = 100 - 100 / (1 + rs);
    }

    for (int i=period;i < len; ++i) {
        double upval;
        double downval;
        double delta = deltas[i-1];

        if (delta > 0) {
            upval = delta;
            downval = 0;
        }
        else if (delta < 0){
            upval = 0;
            downval = -delta;
        }
        else
            upval = downval = 0;

        up = (up * (period - 1) + upval) / period;
        down = (down * (period -1) + downval) / period;

        rs = up / down;

        rsi[i] = 100 - 100 / (1 + rs);
    }
    return rsi;
}

#endif // HELPERS_H
















































