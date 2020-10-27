/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "CatchShow.h"

#include <QCoreApplication>

CatchShow::CatchShow(QObject* parent)
  : QObject(parent)
{
  QCoreApplication::instance()->installEventFilter(this);
}

bool CatchShow::eventFilter(QObject* obj, QEvent* event)
{
  if (this->m_callback && event->type() == QEvent::Show) {
    this->m_callback(obj);
  }

  return this->QObject::eventFilter(obj, event);
}

int CatchShow::count() const
{
  return this->m_count;
}
