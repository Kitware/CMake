module;
#include <QObject>
export module Mod;
export import :Part;

export class PrimaryObject : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
signals:
    void primarySignal(int value);
};
