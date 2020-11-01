/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLinkItem.h"

#include <utility> // IWYU pragma: keep

#include "cmGeneratorTarget.h"

cmLinkItem::cmLinkItem() = default;

cmLinkItem::cmLinkItem(std::string n, bool c, cmListFileBacktrace bt)
  : String(std::move(n))
  , Cross(c)
  , Backtrace(std::move(bt))
{
}

cmLinkItem::cmLinkItem(cmGeneratorTarget const* t, bool c,
                       cmListFileBacktrace bt)
  : Target(t)
  , Cross(c)
  , Backtrace(std::move(bt))
{
}

std::string const& cmLinkItem::AsStr() const
{
  return this->Target ? this->Target->GetName() : this->String;
}

bool operator<(cmLinkItem const& l, cmLinkItem const& r)
{
  // Order among targets.
  if (l.Target && r.Target) {
    return l.Target < r.Target;
  }
  // Order targets before strings.
  if (l.Target) {
    return true;
  }
  if (r.Target) {
    return false;
  }
  // Order among strings.
  if (l.String < r.String) {
    return true;
  }
  // Order among cross-config.
  return l.Cross < r.Cross;
}

bool operator==(cmLinkItem const& l, cmLinkItem const& r)
{
  return l.Target == r.Target && l.String == r.String && l.Cross == r.Cross;
}

std::ostream& operator<<(std::ostream& os, cmLinkItem const& item)
{
  return os << item.AsStr();
}

cmLinkImplItem::cmLinkImplItem()
  : cmLinkItem()
{
}

cmLinkImplItem::cmLinkImplItem(cmLinkItem item, bool fromGenex)
  : cmLinkItem(std::move(item))
  , FromGenex(fromGenex)
{
}
