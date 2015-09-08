#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QStringList>

#define TickerId long
#define OrderId  long

class IBClient;
class PairTabPage;

namespace Ui {
class MainWindow;
class PairTabPage;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Ui::MainWindow* getUi() { return ui; }

private slots:
    void on_action_New_triggered();
    void on_actionConnect_To_TWS_triggered();

    void onManagedAccounts(const QByteArray & msg);
    void onIbError(const int id, const int errorCode, const QByteArray errorString);
    void onNextValidId( OrderId orderId);
    void onCurrentTime(long time);
    void onTwsConnected();
    void onTwsConnectionClosed();

private:
    Ui::MainWindow *ui;
    Ui::PairTabPage* ptpui;
    IBClient* m_ibClient;
    QList<PairTabPage*> m_pairTabPages;
    QStringList m_managedAccounts;
};

#endif // MAINWINDOW_H
