#include "orderstablewidget.h"
#include <QContextMenuEvent>
#include <QMenu>

OrdersTableWidget::OrdersTableWidget(QWidget *parent)
    : QTableWidget(parent)
{

}

void OrdersTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_pos = event->pos();

    QMenu m;
    m.addAction("Close Order", this, SLOT(onCloseOrders()));
    m.exec(event->globalPos());
}

void OrdersTableWidget::onCloseOrders()
{
//    QTableWidgetItem* i = itemAt(m_pos);
//    int row = i->row();

}


