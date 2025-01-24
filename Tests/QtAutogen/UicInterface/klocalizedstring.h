
#ifndef KLOCALIZEDSTRING_H
#define KLOCALIZEDSTRING_H

#include <QString>

#ifdef _WIN32
__declspec(dllexport)
#endif
QString
tr2xi18n(char const* text, char const* comment = 0);

#ifdef _WIN32
__declspec(dllexport)
#endif
QString
tr2i18n(char const* text, char const* comment = 0);

#endif
