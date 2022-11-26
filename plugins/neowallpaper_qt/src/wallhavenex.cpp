#include <wallhavenex.h>
#include <mapeditor.h>
#include <pluginobject.h>
#include <yjson.h>

#include <QActionGroup>
#include <QFileDialog>
#include <QDesktopServices>
#include <QInputDialog>

#include <filesystem>
#include <regex>

namespace fs =std::filesystem;

WallhavenExMenu::WallhavenExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback, std::function<const fs::path&()> getCurImg):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback),
  GetCurImage(getCurImg),
  m_ActionGroup(new QActionGroup(this)),
  m_Separator(addSeparator())
{
  m_ActionGroup->setExclusive(true);
  LoadWallpaperTypes();
  LoadMoreActions();
}

WallhavenExMenu::~WallhavenExMenu()
{
  //
}

void WallhavenExMenu::LoadWallpaperTypes()
{
  const std::u8string& curType = m_Data[u8"WallhavenCurrent"].getValueString();
  for (auto& [u8TypeName, data] :m_Data[u8"WallhavenApi"].getObject()) {
    QAction* const action = new QAction(QString::fromUtf8(u8TypeName.data(), u8TypeName.size()), this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    action->setChecked(u8TypeName == curType);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
  }
}

void WallhavenExMenu::LoadSubSettingMenu(QAction* action)
{
  QMenu* pSonMenu = new QMenu(this);
  pSonMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  action->setMenu(pSonMenu);
  connect(pSonMenu->addAction(action->isChecked() ? "刷新此项" : "启用此项"), &QAction::triggered, pSonMenu, [this, action]() {
    action->setChecked(true);
    QByteArray&& array = action->text().toUtf8();
    m_Data[u8"WallhavenCurrent"].setText(array.begin(), array.end());
    m_CallBack(true);
  });
  if (!action->isChecked()) {
    connect(pSonMenu->addAction("删除此项"), &QAction::triggered, pSonMenu, [pSonMenu, this, action](){
      m_Data[u8"WallhavenApi"].remove(PluginObject::QString2Utf8(action->text()));
      m_CallBack(false);
      pSonMenu->deleteLater();
    });
  }
  connect(pSonMenu->addAction("参数设置"), &QAction::triggered, this,
    std::bind(&WallhavenExMenu::EditCurType, this, PluginObject::QString2Utf8(action->text())));
  connect(pSonMenu->addAction("存储路径"), &QAction::triggered, this, [this, action]() {
    const QString qTypeName = action->text();
    const std::u8string key(PluginObject::QString2Utf8(qTypeName));
    fs::path curdir = m_Data[u8"WallhavenApi"][key][u8"Directory"].getValueString();
    QString qCurDir = QString::fromStdU16String(curdir.u16string());

    qCurDir = QFileDialog::getExistingDirectory(this, "请选择壁纸存放路径", qCurDir);
    if (qCurDir.isEmpty() || qCurDir.isNull()) {
      return;
    }
    curdir = qCurDir.toStdU16String();
    curdir.make_preferred();
    m_Data[u8"WallhavenApi"][key][u8"Directory"] = curdir.u8string();
    m_CallBack(false);
  });
}

void WallhavenExMenu::LoadMoreActions()
{
  connect(addAction("添加更多"), &QAction::triggered, this, std::bind(&WallhavenExMenu::AddNewType, this));
  connect(addAction("关于壁纸"), &QAction::triggered, this, [this]() {
    auto fileName = GetCurImage().stem().string();
    std::regex pattern("^wallhaven-[a-z0-9]{6}$", std::regex::icase);
    if (!std::regex_match(fileName, pattern))
      return;
    std::string url = "https://wallhaven.cc/w/" + fileName.substr(10);
    QDesktopServices::openUrl(QString::fromStdString(url));
  });
  connect(addAction("相似壁纸"), &QAction::triggered, this, [this]() {
    auto fileName = GetCurImage().stem().string();
    std::regex pattern("^wallhaven-[a-z0-9]{6}$", std::regex::icase);
    if (!std::regex_match(fileName, pattern))
      return;
    std::string url =
        "https://wallhaven.cc/search?q=like:" + fileName.substr(10);
    QDesktopServices::openUrl(QString::fromStdString(url));
  });
}

void WallhavenExMenu::AddNewType()
{
  auto& apis = m_Data[u8"WallhavenApi"];
  QString qKeyName = QInputDialog::getText(this, "请输入类型名称", "新名称", QLineEdit::Normal, "新壁纸");
  if (qKeyName.isEmpty()) {
    return;
  }
  std::u8string u8KeyName = PluginObject::QString2Utf8(qKeyName);
  if (apis.find(u8KeyName) != apis.endO()) {
    return;
  }
  const QString folder = QFileDialog::getExistingDirectory(this, "选择壁纸文件夹");
  if (folder.isEmpty() || folder.isNull()) {
    return;
  }
  apis.append(YJson::O {
    {u8"Parameter", YJson::O {}},
    {u8"Directory", PluginObject::QString2Utf8(folder)}
  }, u8KeyName);
  EditCurType(u8KeyName);
}

void WallhavenExMenu::EditCurType(const std::u8string& typeName)
{
  auto& params = m_Data[u8"WallhavenApi"][typeName][u8"Parameter"];
  MapEditor*const editor = new MapEditor("编辑参数", params, [this, typeName](){
    QAction* const action = new QAction( this);
    insertAction(m_Separator, action);
    action->setCheckable(true);
    m_ActionGroup->addAction(action);
    LoadSubSettingMenu(action);
    m_CallBack(typeName == m_Data[u8"WallhavenCurrent"].getValueString());
  });
  editor->show();
}