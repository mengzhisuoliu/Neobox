#include <neobox/pluginmgr.h>
#include <neobox/widgetbase.hpp>
#include <neobox/neosystemtray.hpp>

#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QApplication>

NeoSystemTray::NeoSystemTray()
{
  qApp->setStyleSheet(WidgetBase::GetAppStyle());

  setIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
  setToolTip("Neobox");
  InitDirs();
  InitConnect();
}

NeoSystemTray::~NeoSystemTray()
{
  //
}

void NeoSystemTray::InitDirs()
{
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) {
    dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
  } else {
#if 0
    if (mgr) mgr->glbWriteSharedFlag(1);
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList {});
#endif
    if (mgr) mgr->Quit();
    return;
  }
}

void NeoSystemTray::InitConnect()
{
  connect(this, &QSystemTrayIcon::activated,
    [this](QSystemTrayIcon::ActivationReason reason){
      switch (reason)
      {
        case QSystemTrayIcon::Trigger:
          for (const auto& fun: m_Followers) {
            fun->operator()(PluginEvent::MouseClick, nullptr);
          }
          break;
        case QSystemTrayIcon::DoubleClick:
          for (const auto& fun: m_Followers) {
            fun->operator()(PluginEvent::MouseDoubleClick, nullptr);
          }
          break;
        default:
          break;
      }
    });
}
