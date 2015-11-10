#include "globalconfigdialog.h"
#include "ui_globalconfigdialog.h"
#include <QSettings>
#include <QtDebug>
#include <QKeyEvent>
#include "mainwindow.h"

GlobalConfigDialog::GlobalConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GlobalConfigDialog)
    , mainWindow(qobject_cast<MainWindow*>(parent))

{
    ui->setupUi(this);


    QSettings s;
    s.beginGroup("default");
    ui->timeFrameComboBox_2->setCurrentIndex(s.value("timeFrame").toInt());
    ui->lookbackSpinBox->setValue(s.value("lookback").toInt());
    ui->maPeriodSpinBox->setValue(s.value("maPeriod").toInt());
    ui->rsiPeriodSpinBox->setValue(s.value("rsiPeriod").toInt());
    ui->rsiSpreadSpinBox->setValue(s.value("rsiSpreadPeriod").toInt());
    ui->stdDevPeriodSpinBox->setValue(s.value("stdDevPeriod").toInt());
    ui->volatilityPeriodSpinBox->setValue(s.value("volatilityPeriod").toInt());
    ui->tradeEntryNumStdDevLayersSpinBox_2->setValue(s.value("numStdDevLayers").toInt());
    ui->tradeEntryRSIUpperCheckBox_2->setCheckState((Qt::CheckState)s.value("rsiUpperCheckBoxState").toInt());
    ui->tradeEntryRSIUpperSpinBox_2->setValue(s.value("rsiUpper").toInt());
    ui->tradeEntryRSILowerCheckBox_3->setCheckState((Qt::CheckState)s.value("rsiLowerCheckBoxState").toInt());
    ui->tradeEntryRSILowerSpinBox_3->setValue(s.value("rsiLower").toInt());
    ui->tradeEntryPercentFromMeanCheckBox_2->setCheckState((Qt::CheckState)s.value("percentFromMeanCheckBoxState").toInt());
    ui->tradeEntryPercentFromMeanDoubleSpinBox_2->setValue(s.value("percentFromMean").toDouble());
    ui->tradeEntryAmountSpinBox_2->setValue(s.value("amount").toInt());
    ui->waitCheckBox_2->setCheckState((Qt::CheckState)s.value("waitCheckBoxState").toInt());
    ui->layerBufferCheckBox_2->setCheckState((Qt::CheckState)s.value("bufferCheckBoxState").toInt());
    ui->layerBufferDoubleSpinBox_2->setValue(s.value("buffer").toDouble());
    ui->layerStdDevDoubleSpinBox->setValue(s.value("layerStdDev").toDouble());
    ui->layerTrailCheckBox->setCheckState((Qt::CheckState)s.value("trailCheckBoxState").toInt());
    ui->layerTrailDoubleSpinBox->setValue(s.value("layerTrail").toDouble());
    ui->layerStdMinCheckBox->setCheckState((Qt::CheckState)s.value("layerStdMinCheckBoxState").toInt());
    ui->layerStdMinDoubleSpinBox->setValue(s.value("layerStdMin").toDouble());
    ui->tradeExitPercentStopLossCheckBox_2->setCheckState((Qt::CheckState)s.value("percentStopLossCheckBoxState").toInt());
    ui->tradeExitPercentStopLossDoubleSpinBox_2->setValue(s.value("percentStopLoss").toDouble());
    ui->tradeExitPercentFromMeanCheckBox_2->setCheckState((Qt::CheckState) s.value("tradeExitPercentFromMeanCheckBoxState").toInt());
    ui->tradeExitPercentFromMeanDoubleSpinBox_2->setValue(s.value("tradeExitPercentFromMean").toDouble());
    ui->tradeExitStdDevCheckBox_2->setCheckState((Qt::CheckState)s.value("stdDevExitCheckBoxState").toInt());
    ui->tradeExitStdDevDoubleSpinBox_2->setValue(s.value("stdDevExit").toDouble());
    ui->autoUpdateRangeCheckBox->setCheckState((Qt::CheckState)s.value("autoUpdateRangeCheckBoxState").toInt());
    ui->welcomeCountDownSpinBox->setValue(s.value("welcomeCountDown", 5).toInt());
    s.endGroup();

    ui->lookbackLabel->setVisible(false);
    ui->lookbackSpinBox->setVisible(false);
}

GlobalConfigDialog::~GlobalConfigDialog()
{
    delete ui;
}
Ui::GlobalConfigDialog *GlobalConfigDialog::getUi() const
{
    return ui;
}


void GlobalConfigDialog::on_buttonBox_accepted()
{
    QSettings s;
    s.beginGroup("default");
    s.setValue("timeFrame", ui->timeFrameComboBox_2->currentIndex());
    s.setValue("lookback", ui->lookbackSpinBox->value());
    s.setValue("maPeriod", ui->maPeriodSpinBox->value());
    s.setValue("rsiPeriod", ui->rsiPeriodSpinBox->value());
    s.setValue("rsiSpreadPeriod", ui->rsiSpreadSpinBox->value());
    s.setValue("stdDevPeriod", ui->stdDevPeriodSpinBox->value());
    s.setValue("volatilityPeriod", ui->volatilityPeriodSpinBox->value());
    s.setValue("account", ui->managedAccountsComboBox_2->currentIndex());
    s.setValue("accountString", ui->managedAccountsComboBox_2->currentText());
    s.setValue("amount", ui->tradeEntryAmountSpinBox_2->value());
    s.setValue("rsiUpperCheckBoxState", ui->tradeEntryRSIUpperCheckBox_2->checkState());
    s.setValue("rsiUpper", ui->tradeEntryRSIUpperSpinBox_2->value());
    s.setValue("rsiLowerCheckBoxState", ui->tradeEntryRSILowerCheckBox_3->checkState());
    s.setValue("rsiLower", ui->tradeEntryRSILowerSpinBox_3->value());
    s.setValue("percentFromMeanCheckBoxState", ui->tradeEntryPercentFromMeanCheckBox_2->checkState());
    s.setValue("percentFromMean", ui->tradeEntryPercentFromMeanDoubleSpinBox_2->value());
    s.setValue("numStdDevLayers", ui->tradeEntryNumStdDevLayersSpinBox_2->value());
    s.setValue("waitCheckBoxState", ui->waitCheckBox_2->checkState());
    s.setValue("bufferCheckBoxState", ui->layerBufferCheckBox_2->checkState());
    s.setValue("buffer", ui->layerBufferDoubleSpinBox_2->value());
    s.setValue("layerStdDev", ui->layerStdDevDoubleSpinBox->value());
    s.setValue("trailCheckBoxState", ui->layerTrailCheckBox->checkState());
    s.setValue("layerTrail", ui->layerTrailDoubleSpinBox->value());
    s.setValue("layerStdMinCheckBoxState", ui->layerStdMinCheckBox->checkState());
    s.setValue("layerStdMin", ui->layerStdMinDoubleSpinBox->value());
    s.setValue("percentStopLossCheckBoxState", ui->tradeExitPercentStopLossCheckBox_2->checkState());
    s.setValue("percentStopLoss", ui->tradeExitPercentStopLossDoubleSpinBox_2->value());
    s.setValue("tradeExitPercentFromMeanCheckBoxState", ui->tradeExitPercentFromMeanCheckBox_2->checkState());
    s.setValue("tradeExitPercentFromMean", ui->tradeExitPercentFromMeanDoubleSpinBox_2->value());
    s.setValue("stdDevExitCheckBoxState", ui->tradeExitStdDevCheckBox_2->checkState());
    s.setValue("stdDevExit", ui->tradeExitStdDevDoubleSpinBox_2->value());
    s.setValue("autoUpdateRangeCheckBoxState", ui->autoUpdateRangeCheckBox->checkState());
    s.setValue("welcomeCountDown", ui->welcomeCountDownSpinBox->value());
    s.endGroup();
}

void GlobalConfigDialog::closeEvent(QCloseEvent *evt)
{
    on_buttonBox_accepted();
    QDialog::closeEvent(evt);
}

void GlobalConfigDialog::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);

}
void GlobalConfigDialog::setMangagedAccounts(const QStringList &managedAccounts)
{
    m_managedAccounts = managedAccounts;
    for (int i=0;i<m_managedAccounts.size();++i) {
        ui->managedAccountsComboBox_2->addItem(m_managedAccounts.at(i));
    }
    QSettings s;
    s.beginGroup("default");
//    ui->managedAccountsComboBox_2->setCurrentIndex(s.value("account").toInt());
    //    ui->managedAccountsComboBox_2->setCurrentIndex(s.value("account").toInt());
//qDebug() << "ACCOUNTS.SIZE:" << ui->managedAccountsComboBox_2->count();
    for (int i=0;i<ui->managedAccountsComboBox_2->count();++i) {
        QString txt = ui->managedAccountsComboBox_2->itemText(i);
//qDebug() << "ACCOUNTS:" << txt;
        if (txt == s.value("accountString").toString()) {
            ui->managedAccountsComboBox_2->setCurrentIndex(i);
            break;
        }
    }
    s.endGroup();
}

