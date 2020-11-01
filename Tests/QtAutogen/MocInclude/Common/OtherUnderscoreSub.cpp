#include "OtherUnderscoreSub.hpp"

#include "OtherUnderscoreSubDir/SubExtra.hpp"
#include "OtherUnderscoreSub_p.hpp"

class OtherUnderscoreSubLocal : public QObject
{
  Q_OBJECT
public:
  OtherUnderscoreSubLocal();
  ~OtherUnderscoreSubLocal();
};

OtherUnderscoreSubLocal::OtherUnderscoreSubLocal()
{
}

OtherUnderscoreSubLocal::~OtherUnderscoreSubLocal()
{
}

OtherUnderscoreSubPrivate::OtherUnderscoreSubPrivate()
{
  OtherUnderscoreSubLocal localObj;
  SubExtra extraObj;
}

OtherUnderscoreSubPrivate::~OtherUnderscoreSubPrivate()
{
}

OtherUnderscoreSub::OtherUnderscoreSub()
  : d(new OtherUnderscoreSubPrivate)
{
}

OtherUnderscoreSub::~OtherUnderscoreSub()
{
  delete d;
}

// For OtherUnderscoreSubLocal
#include "OtherUnderscoreSub.moc"
// - Not the own header
// - in a subdirectory
#include "OtherUnderscoreSubDir/moc_SubExtra.cpp"
