#include "welcomedialog.h"
#include "ui_welcomedialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog)
{
    ui->setupUi(this);
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}
Ui::WelcomeDialog *WelcomeDialog::getUi() const
{
    return ui;
}


void WelcomeDialog::on_clearSettingsButton_clicked()
{
    ui->clearSettingsButton->setStyleSheet(QString("QPushButton {background-color: green;}"));
}
