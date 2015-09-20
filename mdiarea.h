#ifndef MDIAREA_H
#define MDIAREA_H

#include <QMdiArea>
#include <QAction>

class MdiArea : public QMdiArea
{
    Q_OBJECT
public:
    explicit MdiArea(QWidget *parent = 0);

    int subWindowWidth() const;
    void setSubWindowWidth(int subWindowWidth);

    int subWindowHeight() const;
    void setSubWindowHeight(int subWindowHeight);

signals:

public slots:

private slots:
    void onCascadeAct();
    void onTabbedAct();
    void onTiledAct();

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

private:
    int m_subWindowWidth;
    int m_subWindowHeight;

    QAction* m_cascadeAct;
    void createActions();
};

#endif // MDIAREA_H
