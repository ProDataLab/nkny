#ifndef ORDERSTABLEWIDGET_H
#define ORDERSTABLEWIDGET_H

#include <QTableWidget>
#include <QPoint>

class OrdersTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit OrdersTableWidget(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:

private slots:
    void onCloseOrders();

private:
    QPoint m_pos;
};

#endif // ORDERSTABLEWIDGET_H
