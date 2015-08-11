#ifndef PAIRTABPAGE_H
#define PAIRTABPAGE_H

#include <QWidget>

namespace Ui {
class PairTabPage;
}

class PairTabPage : public QWidget
{
    Q_OBJECT

public:
    explicit PairTabPage(QWidget *parent = 0);
    ~PairTabPage();

private:
    Ui::PairTabPage *ui;
};

#endif // PAIRTABPAGE_H
