#ifndef MixedSkipped_HPP
#define MixedSkipped_HPP

#include <QObject>

// Object source includes
// - Own moc_ and .moc files.
// - externally generated moc_ file from a SKIP_AUTOMOC enabled header
class MixedSkipped : public QObject
{
  Q_OBJECT
public:
  MixedSkipped();
  ~MixedSkipped();
};

#endif
