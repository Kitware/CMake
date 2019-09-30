#include "SubOwnDot.hpp"

#include "SubOwnDot_p.hpp"

namespace InIncludes {

class SubOwnDotLocal : public QObject
{
  Q_OBJECT
public:
  SubOwnDotLocal();
  ~SubOwnDotLocal();
};

SubOwnDotLocal::SubOwnDotLocal()
{
}

SubOwnDotLocal::~SubOwnDotLocal()
{
}

SubOwnDotPrivate::SubOwnDotPrivate()
{
}

SubOwnDotPrivate::~SubOwnDotPrivate()
{
}

SubOwnDot::SubOwnDot()
{
  SubOwnDotPrivate privateObj;
  SubOwnDotLocal localObj;
}

SubOwnDot::~SubOwnDot()
{
}

} // End of namespace

// For the local QObject
#include "SubOwnDot.moc"
