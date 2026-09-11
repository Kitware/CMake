module;
#include <QObject>
module Mod:Internal;

class InternalObject : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
signals:
    void internalSignal(int value);
};
