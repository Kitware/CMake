#include <QObject>

namespace bar {
  class foo: public QObject {
    Q_OBJECT
  public:
    foo();
    ~foo();
  };
}
