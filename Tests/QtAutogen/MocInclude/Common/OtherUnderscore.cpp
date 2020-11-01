#include "OtherUnderscore.hpp"

#include "OtherUnderscoreExtra.hpp"
#include "OtherUnderscore_p.hpp"

class OtherUnderscoreLocal : public QObject
{
  Q_OBJECT
public:
  OtherUnderscoreLocal();
  ~OtherUnderscoreLocal();
};

OtherUnderscoreLocal::OtherUnderscoreLocal()
{
}

OtherUnderscoreLocal::~OtherUnderscoreLocal()
{
}

OtherUnderscorePrivate::OtherUnderscorePrivate()
{
  OtherUnderscoreLocal localObj;
  OtherUnderscoreExtra extraObj;
}

OtherUnderscorePrivate::~OtherUnderscorePrivate()
{
}

OtherUnderscore::OtherUnderscore()
  : d(new OtherUnderscorePrivate)
{
}

OtherUnderscore::~OtherUnderscore()
{
  delete d;
}

// For OtherUnderscoreLocal
#include "OtherUnderscore.moc"
// - Not the own header
#include "moc_OtherUnderscoreExtra.cpp"
