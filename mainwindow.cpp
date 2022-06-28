#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    deviceDialogUI = new Ui::DeviceDialog;
    deviceDialog = new QDialog(this);
    deviceDialogUI->setupUi(deviceDialog);
    deviceDialogUI->typeComboBox->addItem("Serial Port");
    deviceDialogUI->typeComboBox->addItem("Ethernet Port");
    connect(deviceDialogUI->typeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::getChangedTextSlots);
    connect(deviceDialogUI->portComboBox, &QComboBox::currentTextChanged, this, &MainWindow::ipAddressChangedSlots);
    connect(deviceDialogUI->speedComboBox, &QComboBox::currentTextChanged, this, &MainWindow::ipPortChangedSlost);

    parameterDialogUI = new Ui::ParameterDialog;
    parameterDialog = new QDialog(this);
    parameterDialogUI->setupUi(parameterDialog);

    aboutDialogUI = new Ui::AboutDialog;
    aboutDialog = new QDialog(this,Qt::WindowSystemMenuHint | Qt::WindowTitleHint);

    serialDevice = new SerialDevice();
    netDevice = new NetDevice(); /* Create TCP socket */
    protocol = new Protocol();
    deviceHost = new DeviceHost();

    chartView = new ChartView(ui->chartContainerwidget);
    chartView->setFixedWidth(780);
    chartView->setFixedHeight(480);

    chart = new QChart();

    seriesFacet1 = new QLineSeries();//new QSplineSeries();

    seriesBackGround1 = new QSplineSeries();
    seriesBackGround2 = new QLineSeries();
    seriesBackGround3 = new QLineSeries();

    //bg1
    for(double j=-47.6; j<227.6; j+=0.2)
    {
        tempx = 10*qCos(qDegreesToRadians(j));
        tempy = 10*qSin(qDegreesToRadians(j));
        seriesBackGround1->append(tempx, tempy);
    }
    seriesBackGround1->setColor(QColor(133,188,195));
    chart->addSeries(seriesBackGround1);

    //bg2
    seriesBackGround2->append(0.0,0.0);
    seriesBackGround2->append(10*qCos(qDegreesToRadians(-47.6)),10*qSin(qDegreesToRadians(-47.6)));
    seriesBackGround2->setColor(QColor(133,188,195));
    chart->addSeries(seriesBackGround2);

    //bg3
    seriesBackGround3->append(0.0,0.0);
    seriesBackGround3->append(10*qCos(qDegreesToRadians(227.6)),10*qSin(qDegreesToRadians(227.6)));
    seriesBackGround3->setColor(QColor(133,188,195));
    chart->addSeries(seriesBackGround3);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(-15,15);//(-30,30)
    axisX->setTickCount(7);
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-10,10);//(-20,20)
//    axisY->setTickCount(5);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    seriesBackGround1->attachAxis(axisX);
    seriesBackGround1->attachAxis(axisY);
    seriesBackGround2->attachAxis(axisX);
    seriesBackGround2->attachAxis(axisY);
    seriesBackGround3->attachAxis(axisX);
    seriesBackGround3->attachAxis(axisY);

    chart->legend()->hide();

    chart->addSeries(seriesFacet1);
    seriesFacet1->attachAxis(axisX);
    seriesFacet1->attachAxis(axisY);

    chartView->setRenderHint(QPainter::Antialiasing);

    chartView->setChart(chart);

    ui->actionClose->setDisabled(true);
    ui->actionStop->setDisabled(true);
    ui->actionStart->setEnabled(false);
    ui->actionWms->setDisabled(true);

    lostFrame = 0;
    totalFrame = 0;
    lastFrame = 0;

    record = false;

    file = new QFile();

    receiveTimeOutTimer = new QTimer();
    receiveTimeOutTimer->setInterval(200);

    QObject::connect(receiveTimeOutTimer, &QTimer::timeout, this, &MainWindow::receiveTimeOutSlots, Qt::DirectConnection);
    started = false;

    periodTimer = new QTimer();
    periodTimer->setInterval(5000);
    connect(periodTimer, &QTimer::timeout, this, &MainWindow::periodTimerSlots, Qt::DirectConnection);
    connect(this, &MainWindow::updateTemperature, deviceHost, &DeviceHost::updateTemperature, Qt::DirectConnection);
    connect(deviceHost, &DeviceHost::temperatureSignal, this, &MainWindow::updateTemperatureSlot);

    timeOutCounter = 0;

    connect(this, &MainWindow::stopRawData, deviceHost, &DeviceHost::stopAGV, Qt::DirectConnection);
    connect(this, &MainWindow::updateParameter, deviceHost, &DeviceHost::updateParameter,Qt::DirectConnection);
    connect(this, &MainWindow::storeParameter, deviceHost, &DeviceHost::storeParameter,Qt::DirectConnection);
    connect(this, &MainWindow::startRawData, deviceHost, &DeviceHost::startAGV, Qt::DirectConnection);

    spotsCloud1 = new QVector<QPointF>(1377);


    replayTimer = new QTimer();
    replayTimer->setInterval(40);
    connect(replayTimer, &QTimer::timeout, this, &MainWindow::replayTimerSlots);

    sliderVal = 0;
    wmsStart = 0;
    alg = new Alg;
}


MainWindow::~MainWindow()
{
    delete deviceDialog;

    delete deviceDialogUI;
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    qDebug()<<"Open Device action triggeded.";

    deviceDialogUI->typeComboBox->clear();
    deviceDialogUI->typeComboBox->addItem("Ethernet Port");
    deviceDialogUI->typeComboBox->addItem("Serial Port");

    // For Ethernet communication setting
    deviceDialogUI->label_2->setText("IP Addr:");
    deviceDialogUI->portComboBox->clear();
    deviceDialogUI->portComboBox->addItem("192.168.1.2");
    deviceDialogUI->portComboBox->setEditable(true);

    deviceDialogUI->label_3->setText("Port:");
    deviceDialogUI->speedComboBox->clear();
    deviceDialogUI->speedComboBox->addItem("3050");
    deviceDialogUI->speedComboBox->setEditable(true);

    deviceDialog->exec();

    if(deviceDialog->result())
    {
        serialPortName = deviceDialogUI->portComboBox->itemText(deviceDialogUI->portComboBox->currentIndex());
        serialPortSpeed = deviceDialogUI->speedComboBox->itemText(deviceDialogUI->speedComboBox->currentIndex());

        if(serialPortName != "NULL")
        {
            /* Initial serial port device */
            if(serialPortName.contains("COM") | serialPortName.contains("tty"))
            {
                qDebug()<<"Set up new seiral device";
                qDebug()<<"serialPortName:"<<serialPortName<<", serialPortSpeed:"<<serialPortSpeed;
                if(serialDevice->setDevice(serialPortName, serialPortSpeed))
                {
                    qDebug()<<"MainWindow: setSerialHost";
                    deviceHost->setSerialHost(serialDevice,protocol);
                    /*connect signal and slots*/
                    ui->actionClose->setDisabled(false);
                    ui->actionOpen->setDisabled(true);

                    ui->actionStart->setDisabled(false);
                    ui->actionWms->setDisabled(false);

                    statusLabel.setText(serialPortName);
                    ui->statusBar->addPermanentWidget(&statusLabel);

                }
                else
                {
                    QMessageBox::warning(this,"Error","Device Not Available",QMessageBox::Ok);
                }
            }
            /* Initial Ethernet device */
            else
            {
                qDebug()<<"Set up ethernet device.";
                qDebug()<<"IP: "<<serialPortName;
                qDebug()<<"PORT: "<<serialPortSpeed;

                netIPAddress = deviceDialogUI->portComboBox->itemText(deviceDialogUI->portComboBox->currentIndex());
                netPort = deviceDialogUI->speedComboBox->itemText(deviceDialogUI->speedComboBox->currentIndex());

                /* Create udp socket and connect to host */
                if(netDevice->setClient(netIPAddress, netPort))
                {
                    /* Connect signal and slot */
                    deviceHost->setNetHost(netDevice, protocol);
                    connect(deviceHost, &DeviceHost::newFrame, this, &MainWindow::updateUI);
                    connect(deviceHost, &DeviceHost::newLog, this, &MainWindow::updateLog);

                    ui->actionOpen->setEnabled(false);
                    ui->actionClose->setEnabled(true);
                    ui->actionStart->setEnabled(true);
                    ui->actionStop->setEnabled(true);
                    ui->actionWms->setEnabled(true);

                    statusLabel.setText(netIPAddress + ":" + netPort);
                    ui->statusBar->addPermanentWidget(&statusLabel);

                    periodTimer->start();
                }
                else
                {
                    QMessageBox::warning(this, "Error", "Server Not Available", QMessageBox::Ok);
                }
            }
        }
    }
    else
    {
        qDebug()<<"Do nothing.";
    }

}

void MainWindow::updateUI()
{
    quint16 i;
    static quint16 spotsnum_w = 0;

    qreal radius, angle;
    qreal pointx, pointy;

    if(started == false)
    {
        started = true;
        receiveTimeOutTimer->start();

        ui->receiveStatusLabel->setText("OK!");
        ui->receiveStatusLabel->setStyleSheet("color:green");
    }

    receiveTimeOutTimer->stop();
    receiveTimeOutTimer->start();

    // Confirm spots number
    if(spotsnum_w != deviceHost->MDIFrame_s.frame_s.spotNbr_w)
    {
        spotsnum_w = deviceHost->MDIFrame_s.frame_s.spotNbr_w;
        spotsCloud1->clear();
        spotsCloud1->resize(spotsnum_w); // +2 for start and last render point
    }
    // For AGV RD sensor
    if(spotsnum_w > 0)
    {
        // spotsCloud1->operator[](0) = QPointF(0.0, 0.0);// add start render point to make good link
        for (i = 0; i < spotsnum_w; i++)
        {
            radius = deviceHost->MDIFrame_s.distance_wa[i] / 1000.0; // Should be 1000.0(DEBUG: because the simulator output data < 200)
            // 1) normal = 1000.0
            // 2) output data < 200, then 10.0
            angle = (deviceHost->MDIFrame_s.frame_s.startAngle_sw + \
                    deviceHost->MDIFrame_s.frame_s.deltaAngle_sw / 10 * i) / 100.0;

            pointx = qCos(qDegreesToRadians(angle)) * radius;
            pointy = qSin(qDegreesToRadians(angle)) * radius;
            spotsCloud1->operator[](i) = QPointF(pointx, pointy);
        }
        seriesFacet1->replace(*spotsCloud1);

    }

    totalFrame = deviceHost->totalFrame;

    lostFrame = deviceHost->lostFrame;

    {
        ui->totalFrameLabel->setText(QString::number(totalFrame));
        ui->lostFrameLabel->setText(QString::number(lostFrame));
    }
}

void MainWindow::updateLog()
{

}

void MainWindow::on_actionClose_triggered()
{
    disconnect(deviceHost, &DeviceHost::newFrame, this, &MainWindow::updateUI);

    disconnect(deviceHost, &DeviceHost::newLog, this, &MainWindow::updateLog);

    receiveTimeOutTimer->stop();
    started = false;

    qSleep(1000); // wait for stop rawdata response

    deviceHost->closeNetHost();

    seriesFacet1->clear();

    ui->actionOpen->setEnabled(true);
    ui->actionClose->setEnabled(false);

    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionWms->setEnabled(false);

    ui->totalFrameLabel->setText("0");
    ui->lostFrameLabel->setText("0");
    ui->receiveStatusLabel->setText(" ");
    ui->receiveStatusLabel->setStyleSheet("color:black");
    ui->timeOutLabel->setText(" ");
    ui->ntcLabel->setText(" ");

    periodTimer->stop();
}

void MainWindow::on_actionStart_triggered()
{
    emit startRawData();

    if(started)
    {
        receiveTimeOutTimer->start();
    }
}

void MainWindow::on_actionStop_triggered()
{
    seriesFacet1->clear();

    emit stopRawData();

    receiveTimeOutTimer->stop();
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDialogUI->setupUi(aboutDialog);

    aboutDialog->exec();
}

void MainWindow::on_actionWms_triggered()
{
    wmsdialog = new WmsDialog;

    connect(wmsdialog, &WmsDialog::getWmsSignal, deviceHost, &DeviceHost::getWmsSlot); // WMS dialog -> device host
    connect(deviceHost, &DeviceHost::newWmsSignal, this, &MainWindow::receiveWmsSlot); // device host -> mainwindow
    connect(this, &MainWindow::testWmsSignal, wmsdialog,&WmsDialog::updateWmsSlot);
    connect(this, &MainWindow::updateHpPpSignal, wmsdialog, &WmsDialog::updateWmsResultSlot);

    connect(wmsdialog, &WmsDialog::sendSliderValueSignal, this, &MainWindow::getSliderValSlot);
    connect(wmsdialog, &WmsDialog::sendStartSignal, this, &MainWindow::getWmsStartSlot);

    wmsdialog->exec();

    if(wmsdialog->result() == QDialog::Rejected)
    {
        disconnect(wmsdialog, &WmsDialog::getWmsSignal, deviceHost, &DeviceHost::getWmsSlot);
        disconnect(deviceHost, &DeviceHost::newWmsSignal, this, &MainWindow::receiveWmsSlot);
        disconnect(this, &MainWindow::testWmsSignal, wmsdialog,&WmsDialog::updateWmsSlot);
        disconnect(this, &MainWindow::updateHpPpSignal, wmsdialog, &WmsDialog::updateWmsResultSlot);

        disconnect(wmsdialog, &WmsDialog::sendSliderValueSignal, this, &MainWindow::getSliderValSlot);
        disconnect(wmsdialog, &WmsDialog::sendStartSignal, this, &MainWindow::getWmsStartSlot);

        qDebug()<<"Disconnected getWmsSignal and getWmsSlot";
    }
}

void MainWindow::getChangedTextSlots(QString indexName)
{
    qDebug()<<indexName;
    if(indexName.contains("Ethernet"))
    {
        deviceDialogUI->label_2->setText("IP Addr:");
        deviceDialogUI->portComboBox->clear();
        deviceDialogUI->portComboBox->addItem("192.168.1.2");
        deviceDialogUI->portComboBox->setEditable(true);

        deviceDialogUI->label_3->setText("Port:");
        deviceDialogUI->speedComboBox->clear();
        deviceDialogUI->speedComboBox->addItem("3050");
        deviceDialogUI->speedComboBox->setEditable(true);
    }
    else
    {
        deviceDialogUI->portComboBox->clear();
        deviceDialogUI->portComboBox->setEditable(false);
        deviceDialogUI->speedComboBox->clear();
        deviceDialogUI->speedComboBox->setEditable(false);

        deviceDialogUI->label_2->setText("Port:");

        if(QSerialPortInfo::availablePorts().size()>0)
        {
            foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            {
                if(! QSerialPortInfo(info.portName()).isBusy())
                {
                    deviceDialogUI->portComboBox->addItem(info.portName());
                }
                else {
                    deviceDialogUI->portComboBox->addItem("NULL");
                }
            }
        }
        else
        {
            deviceDialogUI->portComboBox->addItem("NULL");
        }

        deviceDialogUI->label_3->setText("Bits:");
        if(deviceDialogUI->portComboBox->itemText(deviceDialogUI->portComboBox->currentIndex()) != "NULL")
        {
            deviceDialogUI->speedComboBox->clear();
            deviceDialogUI->speedComboBox->addItem("921600");
            deviceDialogUI->speedComboBox->addItem("461200");
            deviceDialogUI->speedComboBox->addItem("230600");
            deviceDialogUI->speedComboBox->addItem("115200");
        }

    }
}

void MainWindow::ipAddressChangedSlots(QString ipaddr)
{
    qDebug()<<ipaddr;
    deviceDialogUI->portComboBox->setItemText(deviceDialogUI->portComboBox->currentIndex(), ipaddr);
}

void MainWindow::ipPortChangedSlost(QString ipport)
{
    qDebug()<<ipport;
    deviceDialogUI->speedComboBox->setItemText(deviceDialogUI->speedComboBox->currentIndex(), ipport);
}

void MainWindow::updateParameterLabel()
{
}

void MainWindow::receiveTimeOutSlots()
{
    //qDebug()<<"Recevie timeout signal. No frame received in 200ms.";
    ui->receiveStatusLabel->setText("TimeOut!");
    ui->receiveStatusLabel->setStyleSheet("color:red");
    timeOutCounter ++;
    ui->timeOutLabel->setText(QString::number(timeOutCounter));
}

void MainWindow::periodTimerSlots()
{
    emit updateTemperature();
}

void MainWindow::receiveWmsSlot(quint16 *buf_, quint16 size_)
{
    QTime elapsedTime;
    //int t1,t2,t3;

    if((wmsStart == 1) && (size_ == WMS_DATA_SIZE))
    {
        if(sliderVal == 0)
        {
            emit testWmsSignal(buf_, WMS_DATA_SIZE, false);
        }
        else if(sliderVal == 1)
        {
            alg->dilation(buf_, dilatedData, WMS_DATA_SIZE);
            emit testWmsSignal(dilatedData, WMS_DATA_SIZE, false);
        }
        else if(sliderVal == 2)
        {
            alg->dilation(buf_, dilatedData, WMS_DATA_SIZE);
            alg->spatialDiscri(dilatedData, spatialData, WMS_DATA_SIZE);
            emit testWmsSignal(spatialData, SPATIAL_DATA_SIZE, true);
        }
        else if(sliderVal == 3)
        {
            elapsedTime.start();
            alg->dilation(buf_, dilatedData, WMS_DATA_SIZE);
            alg->spatialDiscri(dilatedData, spatialData, WMS_DATA_SIZE);
            alg->lzrAttenuation(spatialData, SPATIAL_DATA_SIZE);

            emit testWmsSignal(spatialData, SPATIAL_DATA_SIZE, true);
            emit updateHpPpSignal(alg->HPtx_Idx, alg->HPtx_MaxVal, alg->PPtx_Idx, alg->PPtx_MaxVal, alg->HPrx_Idx, alg->HPrx_MaxVal, alg->PPrx_Idx, alg->PPrx_MaxVal);

        }
    }
}

void MainWindow::calcSpots(quint16 &min, quint16 &max, quint16 &mean, quint16 *buf)
{
    quint32 temp;

    temp = 0;

    min = buf[0];
    max = buf[0];

    for(int i=0; i<MEASUREBUFLEN; i++)
    {
        if(buf[i] > max)
        {
            max = buf[i];
        }
        if(buf[i]< min)
        {
            min = buf[i];
        }

        temp = temp + buf[i];
    }

    mean = static_cast<quint16>(temp / MEASUREBUFLEN);
}

void MainWindow::updateMeasureLable()
{

}

void MainWindow::on_actionParameter_triggered()
{

}

void MainWindow::qSleep(qint32 time)
{
    QTimer::singleShot(time, &eventloop, SLOT(quit()));
    eventloop.exec();
}

void MainWindow::replayTimerSlots()
{
    qreal radius, angle;
    qreal pointx, pointy;

    if((replayCurrentLine + maxSpotsNbr) < totalLines)
    {
        for(int i=0; i<maxSpotsNbr; i++)
        {
            oneLine = textStream->readLine();
            replayCurrentLine ++;

            auto parts = oneLine.split(';');

            if(parts.at(2).data()->isNumber())
            {
                angle = static_cast<qreal>(parts.at(2).toFloat());
                radius = parts.at(3).toInt() / 1000.0;
                pointx = qCos(qDegreesToRadians(angle*2)) * radius;
                pointy = qSin(qDegreesToRadians(angle*2)) * radius;
                spotsCloud1->operator[](i) = QPointF(pointx,pointy);
            }
        }
        seriesFacet1->replace(*spotsCloud1);
    }
    else
    {
        replayTimer->stop();
        qDebug()<<"Stop Re-Play record file.";
        qSleep(1000);

        textStream->flush();
        delete textStream;

        recordFile->close();
        recordFileEnd = false;
        recordFile->remove();
        recordFile->flush();
        delete  recordFile;

    }
}

void MainWindow::on_actionChangeLog_triggered()
{
    QDesktopServices desktopServices;
    QString strUrl = QApplication::applicationDirPath();
    strUrl = QString(QString::fromLocal8Bit("file:///%1/ChangeLog.txt")).arg(strUrl);
    QUrl url(strUrl);
    desktopServices.openUrl(url);
}

void MainWindow::getSliderValSlot(int val_)
{
    sliderVal = val_;
}

void MainWindow::getWmsStartSlot(int val_)
{
    wmsStart = val_;
}

void MainWindow::updateTemperatureSlot(void)
{
    ui->ntcLabel->setText((QString::number(0.01*deviceHost->temperature)));
    if(deviceHost->temperature > 5000)
    {
        ui->ntcLabel->setStyleSheet("color:red");
    }
    else
    {
        ui->ntcLabel->setStyleSheet("color:green");
    }
}
