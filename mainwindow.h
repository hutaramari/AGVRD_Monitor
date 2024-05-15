#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCharts>
#include <QScatterSeries>
using namespace QtCharts;

#include <QMainWindow>
#include <QMessageBox>
#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QVector>
#include <QPointF>
#include <QTableView>
#include <QDesktopServices>
#include <QTime>

#include "ui_mainwindow.h"
#include "ui_devicedialog.h"
#include "ui_aboutdialog.h"
#include "ui_parameterdialog.h"
#include "ui_wmsdialog.h"

#include "serialdevice.h"
#include "netdevice.h"
#include "protocol.h"
#include "devicehost.h"
#include "chartview.h"
#include "wmsdialog.h"
#include "alg.h"

#define MEASUREBUFLEN 100
#define DEBUG 0

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void stopRawData();
    void updateParameter();
    void storeParameter();
    void startRawData();
    void updateTemperature();
    void testWmsSignal(quint16 *buf_, quint16 size_, bool threshEnable_);
    void updateHpPpSignal(quint16 thSect, quint16 thVal, quint16 tpSect, quint16 tpVal,quint16 rhSect, quint16 rhVal,quint16 rpSect, quint16 rpVal);

public slots:

private slots:
    void on_actionOpen_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionClose_triggered();

    void on_actionAbout_triggered();

    void on_actionChangeLog_triggered();

    void on_actionWms_triggered();

    void updateUI(void);
    void updateLog(void);

    void getChangedTextSlots(QString indexName);
    void ipAddressChangedSlots(QString ipaddr);
    void ipPortChangedSlost(QString ipport);

//    void on_actionRecord_triggered();

    void receiveTimeOutSlots();

    void on_actionParameter_triggered();

//    void on_actionPlay_triggered();

    void replayTimerSlots();

    void periodTimerSlots();

    void getSliderValSlot(int val_);
    void getWmsStartSlot(int val_);

    void receiveWmsSlot(quint16 *buf_, quint16 size_);

    void updateTemperatureSlot(void);

    void on_actionReverse_triggered(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    Ui::DeviceDialog *deviceDialogUI;
    Ui::AboutDialog *aboutDialogUI;
    Ui::ParameterDialog *parameterDialogUI;

    QDialog *deviceDialog;
    QDialog *parameterDialog;
    QDialog *aboutDialog;

    QString serialPortName;
    QString serialPortSpeed;
    QString netIPAddress;
    QString netPort;

    SerialDevice *serialDevice;
    NetDevice *netDevice;
    Protocol *protocol;
    DeviceHost *deviceHost;

    ChartView *chartView;
    QChart *chart;

//    QSplineSeries *seriesFacet1;
    QLineSeries *seriesFacet1;

    QSplineSeries *seriesBackGround1;
    QLineSeries   *seriesBackGround2;
    QLineSeries   *seriesBackGround3;

    qreal tempx, tempy;

    QLabel statusLabel;
    QLabel posLabel;
    quint64 totalFrame;
    quint64 lostFrame;
    quint16 maxSpotsNbr;
    quint64 timeOutCounter;

    bool record;

    QFile *file;
    QFile *recordFile;
    QTextStream *textStream;
    QTimer *replayTimer;
    bool recordFileEnd;
    QString oneLine;
    qint64 beginningPos;
    quint64 totalLines;
    quint64 replayCurrentLine;
    quint64 replayFrames;
    quint16 replayMaxSpots;


    float startAngle1;

    QTimer *receiveTimeOutTimer;
    bool started;

    void updateParameterLabel();


    void calcSpots(quint16 &min, quint16 &max, quint16 &mean, quint16 buf[]);
    void updateMeasureLable();

    void qSleep(qint32 time);

    bool parameterOK;

    qint32 totalSpots;

    qint32 dataWidth;

    qint32 uartSpeed;

    float chargeRate;

    QEventLoop eventloop;

    QVector<QPointF> *spotsCloud1;

    QTimer *periodTimer;

    WmsDialog *wmsdialog;

    quint16 wmsTestData[WMS_DATA_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    395, 395, 395, 216, 57, 0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,
    0,   0,   0,   6,   58, 181, 213, 213, 222, 222, 222, 214, 88, 7,   0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   3,   52, 200, 407, 407, 407,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    quint16 dilatedData[WMS_DATA_SIZE];
    quint16 spatialData[SPATIAL_DATA_SIZE];

    int sliderVal;
    int wmsStart;
    Alg *alg;
#if 0
    QVector< QVector<quint16> > Test_sector;
    QVector< QVector<quint16> > Testresult_sector;
    QVector< QVector<quint16> > Testresult_mask;
#endif
    quint8 reverse_b;
    QList<QPointF> tempLine1, tempLineR1;
    QList<QPointF> tempLine2, tempLineR2;
    QList<QPointF> tempLine3, tempLineR3;

    void updateBackground(bool dir_);
};

#endif // MAINWINDOW_H
