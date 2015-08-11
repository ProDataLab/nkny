#include "pairtabpage.h"
#include "ui_pairtabpage.h"

PairTabPage::PairTabPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PairTabPage)
{
    ui->setupUi(this);
}

PairTabPage::~PairTabPage()
{
    delete ui;
}
