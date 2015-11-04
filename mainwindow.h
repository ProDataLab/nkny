#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "globalconfigdialog.h"
#include "ibticktype.h"


#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <QSettings>
#include <QTimer>

#define TickerId long
#define OrderId  long



class IBClient;
class PairTabPage;
struct Order;
struct OrderState;
struct Contract;
class WelcomeDialog;


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

public slots:
    void onOrderStatus(long orderId, const QByteArray &status, int filled,
                                        int remaining, double avgFillPrice, int permId, int parentId,
                                        double lastFillPrice, int clientId, const QByteArray& whyHeld);
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

    void onOpenOrder(long orderId, const Contract& contract, const Order& order, const OrderState& orderState);
    void onOpenOrderEnd();
    void onUpdatePortfolio( const Contract& contract, int position,
       double marketPrice, double marketValue, double averageCost,
       double unrealizedPNL, double realizedPNL, const QByteArray& accountName);
    void onUpdateAccountTime(const QByteArray& timeStamp);

    void onAccountDownloadEnd(const QByteArray & accountName);

    void on_actionGlobal_Config_triggered();

    void onOrdersTableContextMenuEventTriggered(const QPoint & pos, const QPoint & globalPos);
    void onCloseOrder();
    void onHomeTabMoved(int from, int to);
    void onSaveSettings();
    void onTickPrice(const TickerId & tickerId, const TickType & field, const double & price, const int & canAutoExecute);
    void onTickSize(const TickerId & tickerId, const TickType & field, const int & size);
    void onWelcome();
    void onWelcomeTimeout();
    void onClearSettings();
    void onClickShowButtonsManually();



private:
    Ui::MainWindow *ui;
    Ui::PairTabPage* ptpui;
    IBClient* m_ibClient;
    QMap<int,PairTabPage*> m_pairTabPageMap;
    QStringList m_managedAccounts;
    QStringList m_headerLabels;
    QStringList m_orderHeaderLabels;
    QStringList m_portfolioHeaderLabels;
    QSettings m_settings;
    GlobalConfigDialog m_globalConfigDialog;
    QPoint m_ordersTableRowPoint;
    QTimer      m_saveSettingsTimer;
    WelcomeDialog* m_welcomeDialog;
    int             m_numConnectionAttempts;

    void writeSettings();
    void readSettings();
    void readPageSettings();
    void updateOrdersTable(const QString & symbol, const double & last);
};

#endif // MAINWINDOW_H
