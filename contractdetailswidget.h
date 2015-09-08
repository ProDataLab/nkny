#ifndef CONTRACTDETAILSWIDGET_H
#define CONTRACTDETAILSWIDGET_H

#include <QWidget>

class PairTabPage;

namespace Ui {
class ContractDetailsWidget;
}

class ContractDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContractDetailsWidget(QWidget *parent = 0);
    ~ContractDetailsWidget();

    Ui::ContractDetailsWidget *getUi() const;

private slots:
    void on_symbolLineEdit_textEdited(const QString &arg1);
    void on_primaryExchangeLineEdit_textEdited(const QString &arg1);

    void on_securityTypeComboBox_currentIndexChanged(const QString &arg1);

    void on_localSymbolLineEdit_textChanged(const QString &arg1);

    void on_exchangeLineEdit_textChanged(const QString &arg1);

private:
    Ui::ContractDetailsWidget *ui;
    PairTabPage* m_pairTabPage;
};

#endif // CONTRACTDETAILSWIDGET_H
