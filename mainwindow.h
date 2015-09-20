#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "globalconfigdialog.h"

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <QSettings>

#define TickerId long
#define OrderId  long



class IBClient;
class PairTabPage;
class Order;
class OrderState;
class Contract;



namespace Ui {
class MainWindow;
class PairTabPage;
class GlobalConfigDialog;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Ui::MainWindow* getUi() { return ui; }

    QStringList getHeaderLabels() const;

    QStringList getManagedAccounts() const;

    GlobalConfigDialog* getGlobalConfigDialog() const;

    QStringList getOrderHeaderLabels() const;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_action_New_triggered();
    void on_actionConnect_To_TWS_triggered();

    void onManagedAccounts(const QByteArray & msg);
    void onIbError(const int id, const int errorCode, const QByteArray errorString);
    void onIbSocketError(const QString & error);
    void onNextValidId( OrderId orderId);
    void onCurrentTime(long time);
    void onTwsConnected();
    void onTwsConnectionClosed();
    void onTabCloseRequested(int idx);
    void onOrderStatus(long orderId, const QByteArray &status, int filled,
                                        int remaining, double avgFillPrice, int permId, int parentId,
                                        double lastFillPrice, int clientId, const QByteArray& whyHeld);
    void onOpenOrder(long orderId, const Contract& contract, const Order& order, const OrderState& orderState);

    void on_actionGlobal_Config_triggered();


private:
    Ui::MainWindow *ui;
    Ui::PairTabPage* ptpui;
    IBClient* m_ibClient;
    QMap<int,PairTabPage*> m_pairTabPageMap;
    QStringList m_managedAccounts;
    QStringList m_headerLabels;
    QStringList m_orderHeaderLabels;
    QSettings m_settings;
    GlobalConfigDialog m_globalConfigDialog;

    void writeSettings();
    void readSettings();
    void readPageSettings();
};

#endif // MAINWINDOW_H
