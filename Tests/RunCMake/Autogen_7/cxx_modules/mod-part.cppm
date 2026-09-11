module;
#include <QObject>
export module Mod:Part;

export class PartObject : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
signals:
    void partSignal(int value);
};
