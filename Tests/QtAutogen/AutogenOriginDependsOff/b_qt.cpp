
#include "b_qt.hpp"

#include <b_mc.hpp>

namespace b_qt {

/// @brief A source local QObject based class
class Source_QObject : public QObject
{
  Q_OBJECT
public:
  Source_QObject() {}
  ~Source_QObject() {}

  std::string str;
};

std::string mocs_compilation()
{
  // Create and destroy QObject based classes
  Header_QObject header_obj;
  Source_QObject source_obj;

  return std::string(b_mc::mocs_compilation());
}
}

#include "b_qt.moc"
