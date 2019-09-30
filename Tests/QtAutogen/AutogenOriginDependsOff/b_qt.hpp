#ifndef B_QT_HPP
#define B_QT_HPP

#include <string>

#include <config.hpp>

#include <QObject>

namespace b_qt {

/// @brief A header local QObject based class
class Header_QObject : public QObject
{
  Q_OBJECT
public:
  Header_QObject() {}
  ~Header_QObject() {}

  std::string str;
};

/// @brief Function that returns the content of mocs_compilation.cpp
extern std::string mocs_compilation();
}

#endif
