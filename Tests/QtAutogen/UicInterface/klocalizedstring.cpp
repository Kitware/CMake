
#include "klocalizedstring.h"

QString tr2xi18n(char const* text, char const*)
{
  return QLatin1String("TranslatedX") + QString::fromLatin1(text);
}

QString tr2i18n(char const* text, char const*)
{
  return QLatin1String("Translated") + QString::fromLatin1(text);
}
