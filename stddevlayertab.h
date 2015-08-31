#ifndef STDDEVLAYERTAB_H
#define STDDEVLAYERTAB_H

#include <QWidget>

namespace Ui {
class StdDevLayerTab;
}

class StdDevLayerTab : public QWidget
{
    Q_OBJECT

public:
    explicit StdDevLayerTab(int tabIndex, QWidget *parent = 0);
    ~StdDevLayerTab();

    int tabIndex() const { return m_tabIndex; }


    Ui::StdDevLayerTab *getUi() const;

private slots:


    void on_layerTrailCheckBox_stateChanged(int arg1);

private:
    Ui::StdDevLayerTab *ui;
    int m_tabIndex;

};

#endif // STDDEVLAYERTAB_H
