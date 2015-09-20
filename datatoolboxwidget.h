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

private:
    Ui::DataToolBoxWidget *ui;
};

#endif // DATATOOLBOXWIDGET_H
