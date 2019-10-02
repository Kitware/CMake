
#include "a_qt.hpp"

#include <a_mc.hpp>

namespace a_qt {

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

  return std::string(a_mc::mocs_compilation);
}
}

#include "a_qt.moc"
