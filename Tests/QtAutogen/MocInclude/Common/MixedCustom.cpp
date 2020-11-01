#include "MixedCustom.hpp"

class MixedCustomLocal : public QObject
{
  Q_OBJECT

public:
  MixedCustomLocal();
  ~MixedCustomLocal();
};

MixedCustomLocal::MixedCustomLocal()
{
}

MixedCustomLocal::~MixedCustomLocal()
{
}

MixedCustom::MixedCustom()
{
  MixedCustomLocal local;
}

MixedCustom::~MixedCustom()
{
}

// AUTOMOC generated source moc
#include "MixedCustom.moc"
// Externally generated header moc
#include "MixedCustom_extMoc.cpp"
