#ifndef GLBOBJECT_H
#define GLBOBJECT_H

#include <string>

struct GlbObject {
private:
  class NeoSystemTray* glbTray;
  class NeoMenu* glbMenu;
  // class QSharedMemory*const glbSharedMemory;
  class NeoMsgDlg* glbMsgDlg;
public:
  explicit GlbObject();
  ~GlbObject();
#if 0
  bool glbCreateSharedMemory();
  void glbDetachSharedMemory();
  void glbWriteSharedFlag(int flag);
  int glbReadSharedFlag();
#endif
  class QMenu* glbGetMenu();
  class NeoSystemTray* glbGetSystemTray();
  void glbShowMsg(class QString text);
  void glbShowMsgbox(const std::u8string& title, const std::u8string& text, int type = 0);
};

extern GlbObject *glb;

#endif // GLBOBJECT_H
