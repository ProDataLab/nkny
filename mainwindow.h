#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

#define TickerId long
#define OrderId  long

class IBClient;
class PairTabPage;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_New_triggered();
    void on_actionConnect_To_TWS_triggered();

    void onManagedAccounts(const QByteArray & msg);
    void onIbError(const int id, const int errorCode, const QByteArray errorString);
    void onNextValidId( OrderId orderId);
    void onCurrentTime(long time);



private:
    Ui::MainWindow *ui;
    IBClient* m_ibClient;
    QList<PairTabPage*> m_pairTabPages;
};

#endif // MAINWINDOW_H
