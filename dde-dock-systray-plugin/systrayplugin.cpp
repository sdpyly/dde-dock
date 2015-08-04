#include <QDBusConnection>
#include <QWidget>

#include "systrayplugin.h"
#include "compositetrayitem.h"

static const QString CompositeItemKey = "composite_item_key";

SystrayPlugin::SystrayPlugin()
{
    m_compositeItem = new CompositeTrayItem;
}

SystrayPlugin::~SystrayPlugin()
{
    m_compositeItem->deleteLater();
}

void SystrayPlugin::init(DockPluginProxyInterface * proxy)
{
    m_proxy = proxy;
    m_compositeItem->setMode(proxy->dockMode());

    if (!m_dbusTrayManager) {
        m_dbusTrayManager = new com::deepin::dde::TrayManager("com.deepin.dde.TrayManager",
                                                              "/com/deepin/dde/TrayManager",
                                                              QDBusConnection::sessionBus(),
                                                              this);
        connect(m_dbusTrayManager, &TrayManager::Added, this, &SystrayPlugin::onAdded);
        connect(m_dbusTrayManager, &TrayManager::Removed, this, &SystrayPlugin::onRemoved);
    }

    QList<uint> trayIcons = m_dbusTrayManager->trayIcons();
    qDebug() << "Found trayicons: " << trayIcons;

    foreach (uint trayIcon, trayIcons) {
        onAdded(trayIcon);
    }

    m_proxy->itemAddedEvent(CompositeItemKey);
}

QString SystrayPlugin::name()
{
    return QString("System Tray");
}

QStringList SystrayPlugin::uuids()
{
    return QStringList(CompositeItemKey);
}

QString SystrayPlugin::getTitle(QString)
{
    return "";
}

QWidget * SystrayPlugin::getItem(QString)
{
    return m_compositeItem;
}

QWidget * SystrayPlugin::getApplet(QString)
{
    return NULL;
}

void SystrayPlugin::changeMode(Dock::DockMode newMode, Dock::DockMode)
{
    m_compositeItem->setMode(newMode);
    m_proxy->itemSizeChangedEvent(CompositeItemKey);
}

QString SystrayPlugin::getMenuContent(QString)
{
    return "";
}

void SystrayPlugin::invokeMenuItem(QString, QString, bool)
{

}

// private slots
void SystrayPlugin::onAdded(WId winId)
{
    QString key = QString::number(winId);

    QWidget *item = new QWidget;
    item->resize(Dock::APPLET_CLASSIC_ICON_SIZE,
                 Dock::APPLET_CLASSIC_ICON_SIZE);

    QWindow * win = QWindow::fromWinId(winId);
    QWidget * winItem = QWidget::createWindowContainer(win, item);
    winItem->resize(item->size());

    m_compositeItem->addItem(key, item);

    m_proxy->itemSizeChangedEvent(CompositeItemKey);
}

void SystrayPlugin::onRemoved(WId winId)
{
    QString key = QString::number(winId);

    m_compositeItem->removeItem(key);

    m_proxy->itemSizeChangedEvent(CompositeItemKey);
}
