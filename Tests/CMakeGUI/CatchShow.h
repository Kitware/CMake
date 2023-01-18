/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QWidget>

class CatchShow : public QObject
{
  Q_OBJECT
public:
  CatchShow(QObject* parent = nullptr);

  template <typename T, typename F>
  void setCallback(F&& func);
  bool eventFilter(QObject* obj, QEvent* event) override;
  int count() const;

private:
  std::function<void(QObject* obj)> m_callback;
  int m_count = 0;
};

template <typename T, typename F>
void CatchShow::setCallback(F&& func)
{
  this->m_callback = [this, func](QObject* obj) {
    auto* d = qobject_cast<T*>(obj);
    if (d) {
      QMetaObject::invokeMethod(
        obj,
        [this, func, d]() {
          ++this->m_count;
          func(d);
        },
        Qt::QueuedConnection);
    }
  };
}
