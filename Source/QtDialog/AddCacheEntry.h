
#ifndef AddCacheEntry_h
#define AddCacheEntry_h

#include <QWidget>
#include <QCheckBox>

#include "QCMake.h"
#include "ui_AddCacheEntry.h"

class AddCacheEntry : public QWidget, public Ui::AddCacheEntry
{
  Q_OBJECT
public:
  AddCacheEntry(QWidget* p);

  QString name() const;
  QVariant value() const;
  QString description() const;
  QCMakeCacheProperty::PropertyType type() const;
};

#endif

