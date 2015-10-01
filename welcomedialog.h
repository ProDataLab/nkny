#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

namespace Ui {
class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();

    Ui::WelcomeDialog *getUi() const;

private:
    Ui::WelcomeDialog *ui;
};

#endif // WELCOMEDIALOG_H
