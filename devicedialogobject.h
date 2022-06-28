#ifndef DEVICEDIALOGOBJECT_H
#define DEVICEDIALOGOBJECT_H

#include <QObject>
#include <QDialog>
#include <QDebug>
#include <QString>

class DeviceDialogObject : public QDialog
{
public:
    explicit DeviceDialogObject(QDialog *parent=nullptr);

public slots:
    void getCurrentIndexChanged(const QString &arg);
};

#endif // DEVICEDIALOGOBJECT_H
