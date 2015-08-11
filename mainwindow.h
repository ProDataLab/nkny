#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class IBClient;

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

private:
    Ui::MainWindow *ui;
    IBClient* m_ibClient;
};

#endif // MAINWINDOW_H
