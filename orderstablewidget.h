#ifndef ORDERSTABLEWIDGET2_H
#define ORDERSTABLEWIDGET2_H

#include <QTableWidget>
#include <QPoint>



class OrdersTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit OrdersTableWidget(QWidget *parent = 0);
    ~OrdersTableWidget();

signals:
    void contextMenuEventTriggered(const QPoint & pos, const QPoint & globalPos);


public slots:

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;


private:
    QPoint m_pos;

};

#endif // ORDERSTABLEWIDGET2_H
