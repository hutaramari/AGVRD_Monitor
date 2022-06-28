#include "wmsdialog.h"
#include "ui_wmsdialog.h"
#include "devicehost.h"
#include "alg.h"

WmsDialog::WmsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WmsDialog)
{
    ui->setupUi(this);
    setupDemo(0);
    ui->pushButton_wmsStart->setText("Start");
    ui->vSlider->setEnabled(true);

    // wmsTimer is used for sending "GetWms" command to device every 1s
    wmsTimer = new QTimer;
    wmsTimer->setInterval(300);
    connect(wmsTimer, &QTimer::timeout, this, &WmsDialog::wmsTimeoutSlots);

}

WmsDialog::~WmsDialog()
{
    delete ui;
}

void WmsDialog::on_pushButton_wmsStart_clicked()
{
    if(ui->pushButton_wmsStart->text().contains("Start"))
    {
        ui->pushButton_wmsStart->setText("Stop");
        ui->vSlider->setEnabled(false);
        ui->lineEdit_threshold->setEnabled(false);
        wmsTimer->start();
        emit sendStartSignal(1);
    }
    else
    {
        ui->pushButton_wmsStart->setText("Start");
        ui->vSlider->setEnabled(true);
        ui->lineEdit_threshold->setEnabled(true);
        wmsTimer->stop();
        emit sendStartSignal(0);
    }
}

void WmsDialog::setupDemo(int demoIndex)
{
    switch (demoIndex)
    {
    case 0: setupColorMapInit(ui->customPlot);break;
    default:break;
    }

    ui->customPlot->replot();
}

void WmsDialog::setupColorMapInit(QCustomPlot *customPlot)
{
    customPlot->axisRect()->setupFullAxesBox(true);
    QSharedPointer<QCPAxisTicker> xticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
    QSharedPointer<QCPAxisTicker> yticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
    xticker->setTickCount(17);
    yticker->setTickCount(32);
    customPlot->xAxis->setTicker(xticker);
    customPlot->xAxis->setTickLabelRotation(90);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->grid()->setPen(QPen(QColor(0,0,0)));
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->yAxis->setTicker(yticker);
    customPlot->yAxis->setSubTicks(false);
    customPlot->yAxis->grid()->setPen(QPen(QColor(0,0,0)));
    customPlot->yAxis->grid()->setVisible(true);

    customPlot->xAxis->setRange(QCPRange(0, 17));
    customPlot->yAxis->setRange(QCPRange(0, 32));

    // set up the QCPColorMap:
    QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    int nx = WMS_LED_PULSE;
    int ny = WMS_LED_NUM;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(0, nx), QCPRange(0, ny)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    for(int xIndex=0; xIndex<nx;++xIndex)
    {
        for(int yIndex=0; yIndex<ny; ++yIndex)
        {
            colorMap->data()->setCell(xIndex, yIndex, 0.0);
            //colorMap->data()->setAlpha(xIndex, yIndex, 0);
        }
    }

    // set the color gradient of the color map to one of the presets:
    //colorMap->setGradient(QCPColorGradient::gpCold);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.
    QCPColorGradient gradient;
    gradient.setColorStopAt(0.0, QColor("#FFFFFF"));
    gradient.setColorStopAt(0.5, QColor("#BF444C"));
    gradient.setColorStopAt(1.0, QColor("#008000"));
    colorMap->setGradient(gradient);
    colorMap->setDataRange(QCPRange(0,1000));

    colorMap->setInterpolate(false);

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    //colorMap->rescaleDataRange(true);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    customPlot->rescaleAxes();
}

void WmsDialog::setupColorMapDemo(QCustomPlot *customPlot)
{
  // configure axis rect:
  //customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
  customPlot->axisRect()->setupFullAxesBox(true);
  //customPlot->xAxis->setLabel("x");
  //customPlot->yAxis->setLabel("y");
  QSharedPointer<QCPAxisTicker> xticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
  QSharedPointer<QCPAxisTicker> yticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
  xticker->setTickCount(62);
  yticker->setTickCount(29);
  customPlot->xAxis->setTicker(xticker);
  customPlot->xAxis->setTickLabelRotation(90);
  customPlot->xAxis->setSubTicks(false);
  customPlot->xAxis->grid()->setPen(QPen(QColor(0,0,0)));
  customPlot->xAxis->grid()->setVisible(true);
  customPlot->yAxis->setTicker(yticker);
  customPlot->yAxis->setSubTicks(false);
  customPlot->yAxis->grid()->setPen(QPen(QColor(0,0,0)));
  customPlot->yAxis->grid()->setVisible(true);

  // set up the QCPColorMap:
  QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
  int nx = 309;
  int ny = 28;
  colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
  colorMap->data()->setRange(QCPRange(0, nx), QCPRange(0, ny)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
  // now we assign some data, by accessing the QCPColorMapData instance of the color map:
  for(int xIndex=0; xIndex<nx;++xIndex)
  {
      for(int yIndex=0; yIndex<ny; ++yIndex)
      {
          colorMap->data()->setCell(xIndex, yIndex, 0.0);
      }

  }

  // add a color scale:
  //QCPColorScale *colorScale = new QCPColorScale(customPlot);
  //customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
  //colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
  //colorMap->setColorScale(colorScale); // associate the color map with the color scale
  //colorScale->axis()->setLabel("Magnetic Field Strength");

  // set the color gradient of the color map to one of the presets:
  colorMap->setGradient(QCPColorGradient::gpCold);
  // we could have also created a QCPColorGradient instance and added own colors to
  // the gradient, see the documentation of QCPColorGradient for what's possible.

  colorMap->setInterpolate(false);

  // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
  colorMap->rescaleDataRange(true);

  // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
  //QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
  //customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
  //colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  customPlot->rescaleAxes();
}

void WmsDialog::wmsTimeoutSlots()
{
    emit getWmsSignal();
}

void WmsDialog::updateWmsSlot(quint16* buf_, quint16 size_, bool threshEnable_)
{
    auto *colorMap = static_cast<QCPColorMap *>(ui->customPlot->plottable(0));
    int keySize = colorMap->data()->keySize();
    int valueSize = colorMap->data()->valueSize();
    int thresh = static_cast<int>(ui->lineEdit_threshold->text().toInt());

    if(size_ == keySize*valueSize)
    {
        for(int y = 0; y < valueSize; ++y)
        {
            for(int x = 0; x < keySize; ++x)
            {
                if(threshEnable_)
                {
                    if(buf_[y*keySize+x] == 1000)
                    {
                        colorMap->data()->setCell(x, y, 1000);
                    }
                    else
                    {
                        if(buf_[y*keySize+x] > thresh)
                        {
                            colorMap->data()->setCell(x, y, 500);
                        }
                        else
                        {
                            colorMap->data()->setCell(x, y, 0);
                        }
                    }
                }
                else
                {
                    colorMap->data()->setCell(x, y, static_cast<double>(buf_[y*keySize+x]));
                }
            }
        }
        ui->customPlot->replot();
    }
}

void WmsDialog::changeColorMapRowCollum(int row_, int col_)
{
    int tempcol;

    if(col_>100)
    {
        tempcol = col_/5+1;
    }
    else
    {
        tempcol = col_ + 1;
    }
    QSharedPointer<QCPAxisTicker> xticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
    QSharedPointer<QCPAxisTicker> yticker = QSharedPointer<QCPAxisTicker>(new QCPAxisTicker());
    xticker->setTickCount(tempcol);
    yticker->setTickCount(row_+1);
    ui->customPlot->xAxis->setTicker(xticker);
    ui->customPlot->xAxis->setTickLabelRotation(90);
    ui->customPlot->xAxis->setSubTicks(false);
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor("#000000")));
    ui->customPlot->xAxis->grid()->setVisible(true);
    ui->customPlot->yAxis->setTicker(yticker);
    ui->customPlot->yAxis->setSubTicks(false);
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor("#000000")));
    ui->customPlot->yAxis->grid()->setVisible(true);

    ui->customPlot->xAxis->setRange(QCPRange(0, col_));
    ui->customPlot->yAxis->setRange(QCPRange(0, row_));

    auto *colorMap = static_cast<QCPColorMap *>(ui->customPlot->plottable(0));
    colorMap->data()->setSize(col_, row_);
    colorMap->data()->setRange(QCPRange(0, col_), QCPRange(0, row_));
    for(int x = 0; x < col_; ++x)
    {
        for(int y = 0; y < row_; ++y)
        {
            colorMap->data()->setCell(x, y, 0);
        }
    }

    ui->customPlot->replot();
}

void WmsDialog::on_vSlider_valueChanged(int value)
{
    switch (value) {
    case 0:
    case 1:
        changeColorMapRowCollum(WMS_LED_NUM, WMS_LED_PULSE);
        break;
    case 2:
    case 3:
    case 4:
        changeColorMapRowCollum(TSKIN_SPATIALDISCRILINES, TSKIN_SPATIALDISCRICOLUMNS);
        break;
    default:break;
    }
    emit sendSliderValueSignal(value);
}

void WmsDialog::updateWmsResultSlot(quint16 thSect, quint16 thVal, quint16 tpSect, quint16 tpVal,quint16 rhSect, quint16 rhVal,quint16 rpSect, quint16 rpVal)
{
    ui->labelTxHpSectNum->setText(QString::number(thSect));
    ui->labelTxHpVal->setText(QString::number(thVal));
    ui->labelTxPpSectNum->setText(QString::number(tpSect));
    ui->labelTxPpVal->setText(QString::number(tpVal));

    ui->labelRxHpSectNum->setText(QString::number(rhSect));
    ui->labelRxHpVal->setText(QString::number(rhVal));
    ui->labelRxPpSectNum->setText(QString::number(rpSect));
    ui->labelRxPpVal->setText(QString::number(rpVal));

}
