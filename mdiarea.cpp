#include "mdiarea.h"
#include <QtDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMdiSubWindow>

MdiArea::MdiArea(QWidget *parent)
    : QMdiArea(parent)
{

}

void MdiArea::onCascadeAct()
{
    setViewMode(QMdiArea::SubWindowView);
    cascadeSubWindows();
    QMdiSubWindow* w;
    foreach(w, subWindowList())
        w->resize(m_subWindowWidth, m_subWindowHeight);
}

void MdiArea::onTabbedAct()
{
    setViewMode(QMdiArea::TabbedView);
}

void MdiArea::onTiledAct()
{
    setViewMode(QMdiArea::SubWindowView);
    tileSubWindows();
}

void MdiArea::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "[DEBUG-contextMenuEvent]";
    QMenu menu(this);
    if (viewMode() == QMdiArea::SubWindowView) {
        menu.addAction("Cascade View", this, SLOT(onCascadeAct()));
        menu.addAction("Tabbed Veiw", this, SLOT(onTabbedAct()));
        menu.addAction("Tiled View", this, SLOT(onTiledAct()));
    }
    else {
        menu.addAction("Cascade View", this, SLOT(onCascadeAct()));
        menu.addAction("Tiled View", this, SLOT(onTiledAct()));
    }
    menu.exec(event->globalPos());
}
int MdiArea::subWindowHeight() const
{
    return m_subWindowHeight;
}

void MdiArea::setSubWindowHeight(int subWindowHeight)
{
    m_subWindowHeight = subWindowHeight;
}

int MdiArea::subWindowWidth() const
{
    return m_subWindowWidth;
}

void MdiArea::setSubWindowWidth(int subWindowWidth)
{
    m_subWindowWidth = subWindowWidth;
}


void MdiArea::createActions()
{
    m_cascadeAct = new QAction(tr("&Cascade"), this);
    //    newAct->setShortcuts(QKeySequence::New);
    m_cascadeAct->setStatusTip(tr("Display Plots in cascadingWindows"));
    connect(m_cascadeAct, SIGNAL(triggered()), this, SLOT(onCascadeAct()));
}



