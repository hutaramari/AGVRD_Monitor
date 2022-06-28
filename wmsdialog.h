#ifndef WMSDIALOG_H
#define WMSDIALOG_H

#include <QDialog>
#include <QDebug>
#include "qcustomplot.h"
#include "devicehost.h"

namespace Ui {
class WmsDialog;
}

class WmsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WmsDialog(QWidget *parent = nullptr);
    ~WmsDialog();

    void setupDemo(int demoIndex);

    void setupColorMapDemo(QCustomPlot *customPlot);

    void setupColorMapInit(QCustomPlot *customPlot);

    void changeColorMapRowCollum(int row_, int col_);

signals:

    void getWmsSignal();
    void sendSliderValueSignal(int val_);
    void sendStartSignal(int val_);

public slots:

    void updateWmsSlot(quint16 *buf_, quint16 size_, bool threshEnable_);
    void updateWmsResultSlot(quint16 thSect, quint16 thVal, quint16 tpSect, quint16 tpVal,quint16 rhSect, quint16 rhVal,quint16 rpSect, quint16 rpVal);

private slots:

    void on_pushButton_wmsStart_clicked();

    void wmsTimeoutSlots();

    void on_vSlider_valueChanged(int value);

private:
    Ui::WmsDialog *ui;

    QTimer *wmsTimer;
    QTimer *testTimer;

};

#endif // WMSDIALOG_H
