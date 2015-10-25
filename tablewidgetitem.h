#ifndef TABLEWIDGETITEM_H
#define TABLEWIDGETITEM_H

#include <QTableWidgetItem>
#include <QRegularExpression>

class TableWidgetItem : public QTableWidgetItem
{
public:
    TableWidgetItem(int type=Type)
        : QTableWidgetItem(type) {}
    TableWidgetItem(const QString & text, int type=Type)
        : QTableWidgetItem(text, type) {}

    bool operator <(const QTableWidgetItem & other) const
    {
        QRegularExpression re("-?[0-9]+\\.?[0-9]*");
        QRegularExpressionMatch match1 = re.match(text());
        QRegularExpressionMatch match2 = re.match(other.text());

        if (match1.hasMatch() && match2.hasMatch()) {
            return text().toDouble() < other.text().toDouble();
        }
        else if (!match1.hasMatch() && !match2.hasMatch()){
//            return text() < other.text();
            return QTableWidgetItem(*this) < other;
        }
        return false;
    }

signals:

public slots:
};

#endif // TABLEWIDGETITEM_H
