#include "contractdetailswidget.h"
#include "pairtabpage.h"
#include "mainwindow.h"
#include "ui_contractdetailswidget.h"
#include "ui_pairtabpage.h"

#include <QTabWidget>

ContractDetailsWidget::ContractDetailsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContractDetailsWidget)
{
    ui->setupUi(this);
    m_pairTabPage = (PairTabPage*)parent;
}

ContractDetailsWidget::~ContractDetailsWidget()
{
    delete ui;
}
Ui::ContractDetailsWidget *ContractDetailsWidget::getUi() const
{
    return ui;
}


void ContractDetailsWidget::on_symbolLineEdit_textEdited(const QString &arg1)
{
    ui->symbolLineEdit->setText(arg1.toUpper());

}

void ContractDetailsWidget::on_primaryExchangeLineEdit_textEdited(const QString &arg1)
{
    ui->primaryExchangeLineEdit->setText(arg1.toUpper());
}



void ContractDetailsWidget::on_securityTypeComboBox_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "FUT") {
        ui->expiryLineEdit->setEnabled(true);
        ui->strikeLineEdit->setEnabled(false);
        ui->rightLineEdit->setEnabled(false);
        ui->multiplierLineEdit->setEnabled(false);
    }
    else if (arg1 == "OPT") {
        ui->expiryLineEdit->setEnabled(true);
        ui->strikeLineEdit->setEnabled(true);
        ui->rightLineEdit->setEnabled(true);
        ui->multiplierLineEdit->setEnabled(true);
    }
    else if (arg1 == "STK") {
        ui->expiryLineEdit->setEnabled(false);
        ui->strikeLineEdit->setEnabled(false);
        ui->rightLineEdit->setEnabled(false);
        ui->multiplierLineEdit->setEnabled(false);
    }
}
