#ifndef WIDGET1_H
#define WIDGET1_H

#include <QWidget>

namespace Ui {
class Widget1;
}

class Widget1 : public QWidget
{
  Q_OBJECT
public:
  explicit Widget1(QWidget* parent = nullptr);
  ~Widget1();
public slots:
  void onTextChanged(const QString& text);

private:
  Ui::Widget1* ui;
};

#endif // WIDGET1_H
