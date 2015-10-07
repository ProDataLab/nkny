#include "datatoolboxwidget.h"
#include "ui_datatoolboxwidget.h"

DataToolBoxWidget::DataToolBoxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataToolBoxWidget)
{
    ui->setupUi(this);
    ui->lastCointigrationLabel->setVisible(false);
    ui->lastCointegrationLineEdit->setVisible(false);
}

DataToolBoxWidget::~DataToolBoxWidget()
{
    delete ui;
}
Ui::DataToolBoxWidget *DataToolBoxWidget::getUi() const
{
    return ui;
}

