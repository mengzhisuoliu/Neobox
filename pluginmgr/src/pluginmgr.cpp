#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>
#include <systemapi.h>

#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QActionGroup>

#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

// #define WRITE_LOG(x) writelog((x))
#define WRITE_LOG(x) 

void writelog(std::string data)
{
  std::ofstream file("neobox.log", std::ios::app);
  file.write(data.data(), data.size());
  file.close();
}

PluginMgr* mgr;

PluginMgr::PluginMgr(GlbObject* glb, QMenu* pluginMainMenu):
  m_GlbObject(glb),
  m_SettingFileName(u8"PluginSettings.json"),
  SaveSettings([this](){m_Settings->toFile(m_SettingFileName);}),
  m_MainMenu(pluginMainMenu)
{
  mgr = this;
  if (!fs::exists("plugins")) {
    fs::create_directory("plugins");
  }
  if (fs::exists(m_SettingFileName)) {
    m_Settings = new YJson(m_SettingFileName, YJson::UTF8);
  } else {
    m_Settings = new YJson{ YJson::O{
      { u8"Plugins", YJson::O {
        {u8"neospeedboxplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"网速悬浮"},
          {u8"Url", u8"https://github.com/yjmthu/Neobox/..."},   // zipfile url
        }},
        {u8"neotranslateplg", YJson::O{
          {u8"Enabled", true},
          {u8"FriendlyName", u8"极简翻译"},
          {u8"Url", u8"https://github.com/yjmthu/Neobox/..."},
        }},
        {u8"neoocrplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"文字识别"},
          {u8"Url", u8"https://github.com/yjmthu/Neobox/..."},
        }},
        {u8"neowallpaperplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"壁纸引擎"},
          {u8"Url", u8"https://github.com/yjmthu/Neobox/..."},
        }},
        {u8"neosystemplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"系统控制"},
          {u8"Url", u8"https://github.com/yjmthu/Neobox/..."},
        }},
      }},
      { u8"KeyMap", YJson::O {
        {u8"neotranslateplg", YJson::A {
          YJson::A {u8"toggleVisibility", 0, YJson::A {u8"Shift", u8"Z"}},
        }},
        {u8"neoocrplg", YJson::A {
          YJson::A {u8"screenfetch", 0, YJson::A {u8"Ctrl", u8"Shift", u8"A"}},
        }},
      }},
      { u8"PluginsConfig", YJson::O {
      }},
    }};
  }
}

PluginMgr::~PluginMgr()
{
  for (auto [i, j]: m_PluginPath) {
    if (j) delete i;
    FreeLibrary(reinterpret_cast<HINSTANCE>(j));
  }
  delete m_Settings;
}

void PluginMgr::LoadPlugins(QMenu* settingsMenu)
{
  for (auto& [i, j]: m_Settings->find(u8"Plugins")->second.getObject()) {
    const auto name = i;
    const auto& friendlyName = j[u8"FriendlyName"].getValueString();
    auto const action = settingsMenu->addAction(PluginObject::Utf82QString(friendlyName));
    action->setCheckable(true);
    if (j[u8"Enabled"].isTrue()) {
      auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
      auto& plugin = m_Plugins[name];
      if ((plugin = LoadPlugin(name))) {
        plugin->InitMenuAction();
        action->setChecked(true);
      } else {
        action->setChecked(false);
      }
    } else {
      action->setChecked(false);
    }
    QObject::connect(action, &QAction::triggered, settingsMenu, [this, action, name](bool on){
      auto& plugin = m_Plugins[name];
      m_Settings->find(u8"Plugins")->second[name][u8"Enabled"] = on;
      SaveSettings();
      if (on) {
        if ((plugin = LoadPlugin(name))) {
          plugin->InitMenuAction();
        } else {
          action->setChecked(false);
          m_GlbObject->glbShowMsg("设置失败！");
          return;
        }
      } else {
        FreePlugin(plugin);
      }
      m_GlbObject->glbShowMsg("设置成功！");
    });
  }
  SaveSettings();
  InitBroadcast();
}

PluginObject* PluginMgr::LoadPlugin(const std::u8string& pluginName)
{
  PluginObject* (*newPlugin)(YJson&, PluginMgr*)= nullptr;

#ifdef _DEBUG
  fs::path path = __FILEW__;
  path = path.parent_path().parent_path().parent_path() / "build/plugins";
#else
  fs::path path = u8"plugins";
  path /= pluginName;
  if (!LoadPlugEnv(path)) {
    WRITE_LOG("load env failed\n");
    return nullptr;
  }
#endif
  path /= pluginName + u8".dll";
  if (!fs::exists(path)) {
    WRITE_LOG("file not exsist\n");
    return nullptr;
  }
  path.make_preferred();
  std::wstring wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
  if (!hdll) {
    WRITE_LOG("load lib failed\npath name: " + path.string() + "\n");
    return nullptr;
  }
  newPlugin = reinterpret_cast<decltype(newPlugin)>(GetProcAddress(hdll, "newPlugin"));
  if (!newPlugin) {
    WRITE_LOG("load plugin failed\npath name: " + path.string() + "\n");
    FreeLibrary(hdll);
  }
  auto plugin = newPlugin(m_Settings->find(u8"PluginsConfig")->second[pluginName], this);      // nice
  m_PluginPath[plugin] = hdll;
  return plugin;
}

void PluginMgr::FreePlugin(PluginObject*& plugin)
{
  auto const ptr = plugin;
  plugin = nullptr;
  delete ptr;
  auto& hdll = m_PluginPath[ptr];
  FreeLibrary(reinterpret_cast<HINSTANCE>(hdll));
  hdll = nullptr;
}

void PluginMgr::InitBroadcast()
{
  for (auto& [name, plugin]: m_Plugins) {
    if (!plugin) continue;
    for (const auto& [idol, fun]: plugin->m_Following) {
      auto iter = m_Plugins.find(idol);
      if (iter == m_Plugins.end() || !iter->second) continue;
      iter->second->m_Followers.push_back(&fun);
    }
  }
}

bool PluginMgr::LoadPlugEnv(const fs::path& dir)
{
  if (!fs::exists(dir)) {
    WRITE_LOG("dir not exsist\n");
    return false;
  }
  constexpr auto const varName = L"PATH";
  std::wstring strEnvPaths(GetEnvironmentVariableW(varName, nullptr, 0), L'\0');  
  DWORD const dwSize = GetEnvironmentVariableW(varName, strEnvPaths.data(), strEnvPaths.size());
  strEnvPaths.pop_back();
  if (!strEnvPaths.ends_with(L';'))
    strEnvPaths.push_back(L';');
  auto path = fs::absolute(dir);
  path.make_preferred();
  if (strEnvPaths.find(strEnvPaths) != std::wstring::npos)
    return true;
  strEnvPaths.append(path.wstring());
  strEnvPaths.push_back(L'\0');
  BOOL const bRet = SetEnvironmentVariableW(varName, strEnvPaths.data());  
  return bRet;  
}
