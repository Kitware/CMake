#ifndef DualSubMocked_HPP
#define DualSubMocked_HPP

#include <QObject>

// Header moc file is included by DualSub/Second/Second.cpp
class DualSubMocked : public QObject
{
  Q_OBJECT
public:
  DualSubMocked();
  ~DualSubMocked();
};

#endif
