#ifndef HELPERS_H
#define HELPERS_H

#include <QVector>
#include <QtMath>
#include <cmath>
#include <QtDebug>
#include <QString>

#include <iostream>

void delay(int milliSecondsToWait);
double getMin(const QVector<double> & vec);
double getMax(const QVector<double> & vec);
double getMean(const QVector<double> & vec);
QVector<double> getMA(const QVector<double> & vec, int period);
QVector<double> getExpMA(const QVector<double> & vec, int period);
double getStdDev(const QVector<double> & vec);
QVector<double> getStdDevVector(const QVector<double> & vec, int period);
QVector<double> getRSI(const QVector<double> & vec, int period=14);
QVector<double> getCorrelation(const QVector<double> & pair1, const QVector<double> & pair2);
QVector<double> getRatio(const QVector<double> & vec1, const QVector<double> & vec2);
QVector<double> getDiff(const QVector<double> & vec);
QVector<double> getAbsDiff(const QVector<double> & vec);
QVector<double> getDiff(const QVector<double> & vec1, const QVector<double> & vec2);
QVector<double> getRatioVolatility(const QVector<double> & vec, int period);
//QVector<double> getRatioVolatility(const QVector<double> & ratioOfHighs, const QVector<double> &ratioOfLows, int period);
QVector<double> getPercentFromMA(const QVector<double> & vec, int period);
int getMinSize(int s1, int s2);
QVector<double> getVecTimesScalar(const QVector<double> & vec, double scalar);
double getSum(const QVector<double> & vec);

#define pDebug(errStr) qDebug() << "[DEBUG-" << __func__ << __LINE__ << "]" << (errStr)

#endif // HELPERS_H
















































