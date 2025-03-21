#ifndef WIDGETBASE_HPP
#define WIDGETBASE_HPP

#include <QWidget>

#if defined(MYSHAREDLIB_LIBRARY)
#  define MYSHAREDLIB_EXPORT Q_DECL_IMPORT
#else
#  define MYSHAREDLIB_EXPORT Q_DECL_EXPORT
#endif

class QPushButton;

class MYSHAREDLIB_EXPORT WidgetBase: public QWidget
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;
public:
  explicit WidgetBase(QWidget* parent, bool resizeAble=false, bool stayTop=false);
  virtual ~WidgetBase();
  static QString GetAppStyle();
protected:
  virtual void SaveTopState(bool isTop[[maybe_unused]]) {}
  QPushButton* AddTopButton();
  QPushButton* AddCloseButton();
  QPushButton* AddMinButton();
  class QLabel* AddTitle(QString title);
  void AddScrollBar(class QScrollBar* bar, bool horizontal=false);
  void RemoveScrollBar(class QScrollBar* bar);
  void SetShadowAround(QWidget* widget, int radius=20, QColor col=Qt::black, int dx=0, int dy=0);
  void LoadStyleSheet(const QString& path);
private:
  QPoint m_ConstPos;
  std::vector<class QPushButton*> m_Buttons;
  QLabel* m_Title;
  const bool m_ResizeAble;
  uint8_t pressedArea;
  QRect mouseRect;
  void UpdateBorderRect();
  std::array<QRect, 8> pressedRect;
  std::map<class QScrollBar*, bool> m_ScrollBars;
  bool m_StayTop;
};

#endif // WIDGETBASE_HPP
