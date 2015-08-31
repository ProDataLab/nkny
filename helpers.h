#ifndef HELPERS_H
#define HELPERS_H

#include <QVector>
#include <QtMath>
#include <cmath>

double getMin(const QVector<double> & vec);
double getMax(const QVector<double> & vec);
double getMean(const QVector<double> & vec);
QVector<double> getMA(const QVector<double> & vec, int period);
double getVariance(const QVector<double> & vec);
double getStdDev(const QVector<double> & vec);
QVector<double> getMovingStdDev(const QVector<double> & vec, int period);
QVector<double> getRSI(const QVector<double> & vec, int period=14);
QVector<double> getCorrelation(const QVector<double> & pair1, const QVector<double> & pair2);
QVector<double> getRatio(const QVector<double> & vec1, const QVector<double> & vec2);
QVector<double> getDiff(const QVector<double> & vec);
QVector<double> getVolatility(const QVector<double> & vec, int period);
QVector<double> getPercentFromMean(const QVector<double> & vec);

#endif // HELPERS_H
















































