#include "orderstablewidget.h"
#include <QContextMenuEvent>
#include <QMenu>

OrdersTableWidget::OrdersTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
}

OrdersTableWidget::~OrdersTableWidget()
{
}

void OrdersTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuEventTriggered(event->pos(), event->globalPos());
}


