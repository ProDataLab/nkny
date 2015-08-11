#ifndef IBTAGVALUE_H
#define IBTAGVALUE_H

#include <QByteArray>

struct TagValue
{
    TagValue()
    {}

    TagValue(const QByteArray & tag, const QByteArray & value)
        : tag(tag)
        , value(value) {}

    QByteArray tag;
    QByteArray value;
};

#endif // IBTAGVALUE_H

