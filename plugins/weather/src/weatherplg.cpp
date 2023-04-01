﻿#include <weatherdlg.h>
#include <weatherplg.h>

#define PluginName WeatherPlg
#include <pluginexport.cpp>

PluginName::PluginName(YJson& settings)
  : PluginObject(InitSettings(settings), u8"weatherplg", u8"天气预报")
  , m_WeatherDlg(new WeatherDlg)
{
  InitFunctionMap();
}

PluginName::~PluginName() {
  delete m_WeatherDlg;
}

void PluginName::InitFunctionMap()
{
  m_PluginMethod = {
    {u8"openWindow",
      {u8"开关窗口", u8"打开/关闭 U盘助手窗口", [this](PluginEvent, void*){
      }, PluginEvent::Void}
    },
  };
}

QAction* PluginName::InitMenuAction()
{
  return PluginObject::InitMenuAction();
}

YJson& PluginName::InitSettings(YJson& settings)
{
  return settings;
}
