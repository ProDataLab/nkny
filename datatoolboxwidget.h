#ifndef DATATOOLBOXWIDGET_H
#define DATATOOLBOXWIDGET_H

#include <QWidget>

namespace Ui {
class DataToolBoxWidget;
}

class DataToolBoxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataToolBoxWidget(QWidget *parent = 0);
    ~DataToolBoxWidget();

    Ui::DataToolBoxWidget *getUi() const;

private:
    Ui::DataToolBoxWidget *ui;
};

#endif // DATATOOLBOXWIDGET_H
