#include "stddevlayertab.h"
#include "ui_stddevlayertab.h"

StdDevLayerTab::StdDevLayerTab(int tabIndex, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StdDevLayerTab)
    , m_tabIndex(tabIndex)
{
    ui->setupUi(this);
    ui->layerTrailCheckBox->setEnabled(true);
    ui->layerTrailDoubleSpinBox->setEnabled(true);
}

StdDevLayerTab::~StdDevLayerTab()
{
    delete ui;
}




void StdDevLayerTab::on_layerTrailCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        ui->layerStdMinCheckBox->setEnabled(true);
        ui->layerStdMinDoubleSpinBox->setEnabled(true);

    }
    else {
        ui->layerStdMinCheckBox->setEnabled(false);
        ui->layerStdMinDoubleSpinBox->setEnabled(false);
    }
}
Ui::StdDevLayerTab *StdDevLayerTab::getUi() const
{
    return ui;
}

