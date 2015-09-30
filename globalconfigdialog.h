#ifndef GLOBALCONFIGDIALOG_H
#define GLOBALCONFIGDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class GlobalConfigDialog;
}

class GlobalConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalConfigDialog(QWidget *parent = 0);
    ~GlobalConfigDialog();

    Ui::GlobalConfigDialog *getUi() const;

    void setMangagedAccounts(const QStringList &managedAccounts);

private slots:
    void on_buttonBox_accepted();

protected:
    void closeEvent();
    void keyPressEvent(QKeyEvent * evt);

private:
    Ui::GlobalConfigDialog *ui;
    MainWindow* mainWindow;
    QStringList m_managedAccounts;
};

#endif // GLOBALCONFIGDIALOG_H
