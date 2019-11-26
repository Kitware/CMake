#include "MixedSkipped.hpp"

#include "MixedCustom.hpp"

class MixedSkippedLocal : public QObject
{
  Q_OBJECT

public:
  MixedSkippedLocal();
  ~MixedSkippedLocal();
};

MixedSkippedLocal::MixedSkippedLocal()
{
}

MixedSkippedLocal::~MixedSkippedLocal()
{
}

MixedSkipped::MixedSkipped()
{
  MixedSkippedLocal local;
  MixedCustom externCutom;
  // Call moc named function
  moc_MixedCustom(externCutom);
}

MixedSkipped::~MixedSkipped()
{
}

// Include AUTOMOC generated moc files
#include "MixedSkipped.moc"
#include "moc_MixedSkipped.cpp"

// Include externally generated moc_ named file that is not a moc file
// and for which the relevant header is SKIP_AUTOMOC enabled
#include "moc_MixedCustom.cpp"
