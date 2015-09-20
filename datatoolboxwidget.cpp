#include "datatoolboxwidget.h"
#include "ui_datatoolboxwidget.h"

DataToolBoxWidget::DataToolBoxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataToolBoxWidget)
{
    ui->setupUi(this);
}

DataToolBoxWidget::~DataToolBoxWidget()
{
    delete ui;
}
