#include "helpers.h"
#include <QVector>
#include <QtMath>
#include <QDebug>
#include <QPair>
#include <cmath>
#include <QTime>
#include <QCoreApplication>

double getMin(const QVector<double> & vec)
{
    double min = 1000000;

    for (int i = 0;i < vec.size(); ++i) {
        if (vec[i] < min)
            min = vec[i];
    }
    return min;
}

double getMax(const QVector<double> & vec)
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

QVector<double> getMA(const QVector<double> & vec, int period)
{
    QVector<double> ret;

    for (int i=period;i<vec.size();++i) {
        double sum = 0;
        for (int j=0; j<period;++j) {
//            qDebug() << "[i-j]" << i-j;
            sum += vec.at(i-j);
        }
        ret += sum / period;
    }
//    qDebug() << "[DEBUG-getMA] ret.size:" << ret.size();
    return ret;
}


QVector<double> getStdDevVector(const QVector<double> & vec, int period)
{
    int n = vec.size();
    QVector<double> ret;
    double sum = 0;
    double mean = 0;
    double final = 0;
    double val = 0;

    for (int i=period;i<n-1;++i) {
        sum = 0;
        mean = getMean(vec.mid(i-period, period));
        for (int j = i-period;j<i;++j) {
            val = vec.at(j);
            sum += pow(val - mean, 2);
        }
        final = qSqrt(sum/period);
        double newObservation = vec.at(i+1);
        double deviation = newObservation - mean;

        ret.append(deviation / final);
    }
    return ret;
}



QVector<double> getRSI(const QVector<double> & vec, int period)
{
    // http://stockcharts.com/school/doku.php?id=chart_school:technical_indicators:relative_strength_index_rsi
    double gainSum = 0;
    double lossSum = 0;
    double prevGainAvg = 0;
    double prevLossAvg = 0;
    double gainAvg = 0;
    double lossAvg = 0;
    double rs = 0;
    double rsi = 0;
    QVector<double> ret;


    for (int i=1;i<vec.size();++i) {
        double diff = vec.at(i) - vec.at(i-1);
        if (diff > 0)
            gainSum += diff;
        else if (diff < 0)
            lossSum += diff * -1;
        if (i==period-1) {
            gainAvg = gainSum / period;
            lossAvg = lossSum / period;
            gainSum = lossSum = 0;
        }
        else if (i >= period) {
            gainAvg = (prevGainAvg * (period-1) + gainSum) / period;
            lossAvg = (prevLossAvg * (period-1) + lossSum) / period;
            prevGainAvg = gainAvg;
            prevLossAvg = lossAvg;
            gainSum = lossSum = 0;

            rs = gainAvg / lossAvg;
            gainAvg = lossAvg = 0;
            rsi = 100 - (100 / (1 + rs));
            ret.append(rsi);
        }

    }
    return ret;
}

QVector<double> getCorrelation(const QVector<double> & pair1, const QVector<double> & pair2)
{
    double pMean1 = getMean(pair1);
    double pMean2 = getMean(pair2);

    QVector<double> pair11;
    QVector<double> pair22;

    int size = 0;
    if (pair1.size() <= pair2.size()) {
        size = pair1.size();
        pair11 = pair1;
        pair22 = pair2.mid(pair2.size() - pair1.size());
    }
    else {
        size = pair2.size();
        pair11 = pair1.mid(pair1.size() - pair2.size());
        pair22 = pair2;
    }

    double absum=0;
    double aasum=0;
    double bbsum=0;

    QVector<double> ret;

    for (int i=0;i<size;++i) {
        double a = pair11.at(i) - pMean1;
        double b = pair22.at(i) - pMean2;
        double ab = a * b;
        absum += ab;
        double aa = a * a;
        aasum += aa;
        double bb = b * b;
        bbsum += bb;

        ret.append(absum / sqrt(aasum * bbsum));
    }
    return ret;
}


QVector<double> getRatio(const QVector<double> & vec1, const QVector<double> & vec2)
{
    int size = 0;

    QVector<double> vec11;
    QVector<double> vec22;

    if (vec1.size() < vec2.size()) {
        size = vec1.size();
        vec11 = vec1;
        vec22 = vec2.mid(vec2.size()-size);
    }
    else {
        size = vec2.size();
        vec11 = vec1.mid(vec1.size()-size);
        vec22 = vec2;
    }

    QVector<double> ret(size);


    for (int i=0; i<size;++i) {
        ret[i] = (vec11.at(i) / vec22.at(i));
    }

    return ret;
}

QVector<double> getDiff(const QVector<double> & vec)
{
    QVector<double> ret;
    for (int i=1;i<vec.size();++i) {
        ret.append(vec.at(i) - vec.at(i-1));
    }
    return ret;
}

QVector<double> getRatioVolatility(const QVector<double> & ratio, int period)
{
    QVector<double> ret;
    QVector<double> expMA = getExpMA(getAbsDiff(getVecTimesScalar(ratio, 100)), period);

    for (int i=period;i<expMA.size();++i) {
        ret.append(((expMA.at(i) - expMA.at(i-period)) / expMA.at(i-period)) * 100);
    }
    return ret;
}

QVector<double> getPercentFromMean(const QVector<double> & vec)
{
    double mean = getMean(vec);
    QVector<double> ret(vec.size());
    for (int i = 0;i<vec.size();++i) {
        ret[i] = vec.at(i) / mean * 100;
    }
    return ret;
}


void delay(int milliSecondsToWait)
{
    QTime dieTime = QTime::currentTime().addMSecs( milliSecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}


QVector<double> getDiff(const QVector<double> &vec1, const QVector<double> &vec2)
{
    int mid = 0;
    int size = 0;
    QVector<double> ret;
    QVector<double> vec11;
    QVector<double> vec22;

    if (vec1.size() == vec2.size()) {
        vec11 = vec1;
        vec22 = vec2;
        size = vec1.size();
    }
    else if (vec1.size() < vec2.size()) {
        mid = vec2.size() - vec1.size();
        vec11 = vec1;
        vec22 = vec2.mid(mid);
        size = vec1.size();
    }
    else {
        mid = vec1.size() - vec2.size();
        vec11 = vec1.mid(mid);
        vec22 = vec2;
        size = vec2.size();
    }
    for (int i=0;i<size;++i) {
        ret.append(vec11.at(i) - vec22.at(i));
    }
    return ret;
}


int getMinSize(int s1, int s2)
{
    if (s1 < s2)
        return s1;
    else
        return s2;
}


double getStdDev(const QVector<double> &vec)
{
    double sum = 0;
    double mean = getMean(vec);

    for (int i=0;i<vec.size();++i) {
        sum += pow(vec.at(i) - mean, 2);
    }

    return qSqrt(sum / vec.size());
}


QVector<double> getExpMA(const QVector<double> &vec, int period)
{
    QVector<double> ret;

    double expPercentage = 2 / (period + 1);


    for (int i=period+1;i<vec.size();++i) {
        double val = vec.at(i) * expPercentage;
        double ma = getMean(vec.mid(i-period, i));
        ret.append(val - (ma - (100-expPercentage)));
    }
    return ret;
}

QVector<double> getAbsDiff(const QVector<double> &vec)
{
    QVector<double> ret;
    for (int i=1;i<vec.size();++i) {
        ret.append(fabs(vec.at(i) - vec.at(i-1)));
    }
    return ret;
}


QVector<double> getVecTimesScalar(const QVector<double> &vec, double scalar)
{
    QVector<double> ret;
    for (int i=0;i<vec.size();++i) {
        ret.append(vec.at(i) * scalar);
    }
    return ret;
}
