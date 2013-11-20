
#include "klocalizedstring.h"

QString tr2xi18n(const char *text, const char *)
{
  return QStringLiteral("TranslatedX") + QString::fromLatin1(text);
}

QString tr2i18n(const char *text, const char *)
{
  return QStringLiteral("Translated") + QString::fromLatin1(text);
}
