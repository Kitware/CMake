#ifndef TestClass_hpp
#define TestClass_hpp

#include <QObject>
#include <QtGlobal>
// include qplatformdefs.h for #18669
#include <qplatformdefs.h>

class TestClass : public QObject
{
  Q_OBJECT
public Q_SLOTS:

  // Method named "open" to test if #18669 is fixed
  void open();

// -- Mac
#ifndef Q_OS_MAC
  void MacNotDef();
#else
  void MacNotDefElse();
#endif

#ifdef Q_OS_MAC
  void MacDef();
#else
  void MacDefElse();
#endif

// -- Unix
#ifndef Q_OS_UNIX
  void UnixNotDef();
#else
  void UnixNotDefElse();
#endif

#ifdef Q_OS_UNIX
  void UnixDef();
#else
  void UnixDefElse();
#endif

// -- Windows
#ifndef Q_OS_WIN
  void WindowsNotDef();
#else
  void WindowsNotDefElse();
#endif

#ifdef Q_OS_WIN
  void WindowsDef();
#else
  void WindowsDefElse();
#endif
};

#endif /* TestClass_hpp */
