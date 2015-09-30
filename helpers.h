#ifndef HELPERS_H
#define HELPERS_H

#include <QVector>
#include <QtMath>
#include <cmath>
#include <QtDebug>
#include <QString>

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
QVector<double> getRatioVolatility(const QVector<double> & ratio, int period);
QVector<double> getPercentFromMean(const QVector<double> & vec);
int getMinSize(int s1, int s2);
QVector<double> getVecTimesScalar(const QVector<double> & vec, double scalar);

#define P_DEBUG qDebug() << "[DEBUG]" << /*__PRETTY_FUNCTION__*/ __func__ << __LINE__

#endif // HELPERS_H
















































