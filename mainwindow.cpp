/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011, 2012, 2013, 2014 Emanuel Eichhammer               **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 07.04.14                                             **
**          Version: 1.2.1                                                **
****************************************************************************/

/************************************************************************************************************
**                                                                                                         **
**  This was developed from example code for QCustomPlot.                                                  **
**  UC Davis iGEM 2014                                                                                     **
**                                                                                                         **
**                                                                                                         **
*************************************************************************************************************/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSerialPort>
#include <QSerialPortInfo>

QSerialPort serial;

/*************************************************************************************************************/
/************************************************ CONSTRUCTOR ************************************************/
/*************************************************************************************************************/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowTitle("OliView");
    ui->setupUi(this);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);

    ui->customPlot->xAxis->setRange(0, 1000);
    ui->customPlot->yAxis->setRange(-10, 10);
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");

    setUpComPort();

    fillPortsInfo();

    setupAldeSensGraph(ui->customPlot);
    
    setupWaveTypes();
    
    connect(ui->sampButtonCV, SIGNAL(clicked()), this, SLOT(sampCVPressed()));
    connect(ui->sampButtonPA, SIGNAL(clicked()), this, SLOT(sampPAPressed()));
    connect(ui->sampButtonAS, SIGNAL(clicked()), this, SLOT(sampASPressed()));
    //connect(ui->reconButton, SIGNAL(clicked()), this, SLOT(reconnectButtonPressed()));
    //connect(ui->clrButton, SIGNAL(clicked()), this, SLOT(clearButtonPressed()));
    connect(ui->ASwaveType, SIGNAL(activated(int)), this, SLOT(waveType()));

    connect(ui->action_10_A, SIGNAL(triggered()), this, SLOT(res10ASelected()));
    connect(ui->action_10_nA, SIGNAL(triggered()), this, SLOT(res10nASelected()));
    connect(ui->action_100_nA, SIGNAL(triggered()), this, SLOT(res100nASelected()));
    connect(ui->action_1000_nA, SIGNAL(triggered()), this, SLOT(res1000nASelected()));

    connect(ui->actionClear_All, SIGNAL(triggered()), this, SLOT(clearAllSelected()));
    connect(ui->actionReset_Axis, SIGNAL(triggered()), this, SLOT(resetAxis()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeSelected()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectSelected()));

    connect(ui->action2000_Hz, SIGNAL(triggered()), this, SLOT(rate2000Selected()));
    connect(ui->action5000_Hz, SIGNAL(triggered()), this, SLOT(rate5000Selected()));
    connect(ui->action10000_Hz, SIGNAL(triggered()), this, SLOT(rate10000Selected()));

    sampleRate = 2000;
    waveNum = 0;
}

/*************************************************************************************************************/
/************************** INITIALIZE ALL WAVE TYPES USING ICONS IN DROP DOWN MENU **************************/
/*************************************************************************************************************/

void MainWindow::setupWaveTypes()
{
    QIcon sineWave(":/Images/SineWave.png");
    QIcon triangleWave(":/Images/TriangleWave.png");
    QIcon squareWave(":/Images/SquareWave.png");

    ui->ASwaveType->insertItem(0, squareWave,(const char *) 0);
    ui->ASwaveType->setIconSize(QSize(100,28));
    ui->ASwaveType->insertItem(1, sineWave,(const char *) 0);
    ui->ASwaveType->setIconSize(QSize(100,28));
    ui->ASwaveType->insertItem(2, triangleWave,(const char *) 0);
    ui->ASwaveType->setIconSize(QSize(100,28));
    ui->ASwaveType->setCurrentIndex(0);
}


/*************************************************************************************************************/
/******************************* INITIALIZE ALL SERIAL PORT DATA FOR SAMPLING ********************************/
/*************************************************************************************************************/

void MainWindow::setUpComPort()
{
    serial.setPortName("com6");
    if (serial.open(QIODevice::ReadWrite))
    {
        serial.setBaudRate(QSerialPort::Baud9600);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        ui->statusBar->showMessage(QString("COM Port Successfully Linked"));
        serial.write("changeSampleRate!2000@#$%");                            //Set default Sampling Rate
        serial.write("resolution!20@#$%");                                    //Set default Resolution

    }

    else
    {
        serial.close();
        ui->statusBar->showMessage(QString("Unable to Reach COM Port"));
    }


}

/*************************************************************************************************************/
/************************************ INITIALIZE PROPERTIES OF THE GRAPH *************************************/
/*************************************************************************************************************/

void MainWindow::setupAldeSensGraph(QCustomPlot *customPlot)
{

    // applies a different color brush to the first 9 graphs
    for (int i=0; i<10; ++i) {
        ui->customPlot->addGraph(); // blue line
        ui->customPlot->graph(i)->setAntialiasedFill(false);

        switch (i) {
        case 0: customPlot->graph(i)->setPen(QPen(Qt::blue)); break;
        case 1: customPlot->graph(i)->setPen(QPen(Qt::red)); break;
        case 2: customPlot->graph(i)->setPen(QPen(Qt::green)); break;
        case 3: customPlot->graph(i)->setPen(QPen(Qt::yellow)); break;
        case 4: customPlot->graph(i)->setPen(QPen(Qt::cyan)); break;
        case 5: customPlot->graph(i)->setPen(QPen(Qt::magenta)); break;
        case 6: customPlot->graph(i)->setPen(QPen(Qt::darkRed)); break;
        case 7: customPlot->graph(i)->setPen(QPen(Qt::darkGreen)); break;
        case 8: customPlot->graph(i)->setPen(QPen(Qt::darkBlue)); break;

        }
    }

    customPlot->xAxis->setTickStep(1);
    customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

    // setup policy and connect slot for context menu popup:
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    ui->customPlot->replot();
}

/*************************************************************************************************************/
/********************************* ALLOW USERS TO CLICK AND DRAG THE GRAPH ***********************************/
/*************************************************************************************************************/

void MainWindow::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());

    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());

    else
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

/*************************************************************************************************************/
/********************** ALLOW THE USERS TO SCROLL ON TOP OF THE GRAPH TO ZOOM IN AND OUT *********************/
/*************************************************************************************************************/

void MainWindow::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());

    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());

    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

/*************************************************************************************************************/
/*********************************** GRAPH SELECTION FUNCTIONS ***********************************************/
/*************************************************************************************************************/

//------------------------------------------------------------------------------------Remove One Graph (Selected Graph)

void MainWindow::removeSelectedGraph()
{
    if (ui->customPlot->selectedGraphs().size() > 0)
    {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
        ui->customPlot->replot();
    }
}

//----------------------------------------------------------------------------------------------------Remove All Graphs

void MainWindow::removeAllGraphs()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->replot();
}

//--------------------------------------------------------------------------------Functionality of Right Click on Mouse


void MainWindow::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->customPlot->selectedGraphs().size() > 0)
        menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->customPlot->graphCount() > 0)
        menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));

    menu->popup(ui->customPlot->mapToGlobal(pos));
}

//------------------------------------------------------------------------------------Functionality of Selection Change

void MainWindow::selectionChanged()
{
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<ui->customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = ui->customPlot->graph(i);
        QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelected(true);
        }
    }
}

//--------------------------------------------------------------------------------------Functionality of Clicking Graph

void MainWindow::graphClicked(QCPAbstractPlottable *plottable)
{
    ui->statusBar->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 2000);
}

/*************************************************************************************************************/
/********************* INITIALIZE WAVE TYPE BASED ON USER'S CHOICE FROM DROP DOWN MENU ***********************/
/*************************************************************************************************************/

void MainWindow::waveType()
{
    int index = ui->ASwaveType->currentIndex();

    waveNum = index;
}



/*************************************************************************************************************/
/************************************* FUNCTIONALITY OF 3 SAMPLE BUTTONS *************************************/
/*************************************************************************************************************/

//-----------------------------------------------------------------------------------------------------Anodic Stripping
// Example Instruction "anoStrip0.101.005000,"
//      Start Volt (0.00 Volts)    ASstartVolt  float (must be 3 digits)
//      Peak Volt  (1.00 Volts)    ASpeakVolt   float (must be 3 digits)
//      Scan Rate  (500 mV/S)      ASscanRate   float (must be 3 digits)
//      Wave Type  (  0 - constant )            int   (must be 1 digit)
//                   1 - sin wave
//                   2 - triangle wave
//

void MainWindow::sampASPressed()
{
    QString ASsv = QString::number(ui->ASstartVolt->value(),'f',2);
    QString ASpv = QString::number(ui->ASpeakVolt->value(),'f',2);
    QString ASsr = QString::number(ui->ASscanRate->value());
    QString wave = QString::number(waveNum);

    QString mainInstructions = ("anoStrip!"+ASsv+"@"+ASpv+"#"+ASsr+"$"+wave+"%");
    serial.write(mainInstructions.toStdString().c_str());

    QTimer::singleShot(140, this, SLOT(preParse()));

    //ui->sampButton->setText(QString("Resample"));
}


//---------------------------------------------------------------------------------------------------Cyclic Voltammetry


void MainWindow::sampCVPressed()
{
    QString CVsv = QString::number(ui->CVstartVolt->value(),'f',2);
    QString CVpv = QString::number(ui->CVpeakVolt->value(),'f',2);
    QString CVsr = QString::number(ui->CVscanRate->value());

    QString mainInstructions = ("cycVolt!"+CVsv+"@"+CVpv+"#"+CVsr+"$"+"2%");
    serial.write(mainInstructions.toStdString().c_str());

    QTimer::singleShot(140, this, SLOT(preParse()));;

}

//-------------------------------------------------------------------------------------------Potentiostatic Amperometry

void MainWindow::sampPAPressed()
{
    QString PApv = QString::number(ui->PApotVolt->value(),'f',2);
    QString PAst = QString::number(ui->PAsampTime->value(),'f',2);

    QString mainInstructions = ("potAmpero!"+PAst+"@"+PApv+"#$%");
    serial.write(mainInstructions.toStdString().c_str());

    qDebug() << mainInstructions;

    QTimer::singleShot(140, this, SLOT(preParse()));

}



/*************************************************************************************************************/
/***************************** READ DATA FROM SERIAL PORT AND GRAPH THE VALUES *******************************/
/*************************************************************************************************************/

void MainWindow::preParse() {

    ui->customPlot->clearGraphs();
    ui->statusBar->showMessage(QString("Sampling..."));

    QString sampleBuf = serial.readLine();
    QString voltDivBuf = serial.readLine();
    QString gainBuf = serial.readLine();

    samples = sampleBuf.toInt();                                    //Teensy now sends the number of samples in the first line
    voltDiv = voltDivBuf.toFloat();
    gain = gainBuf.toFloat();

    qDebug() << samples;
    qDebug() << voltDiv;
    qDebug() << gain;
    if ((ui->toolBox2->currentIndex() == 1) || (waveNum == 2 && ui->toolBox2->currentIndex() == 0)) {



        QTimer::singleShot(samples*1000/sampleRate, this, SLOT(CVparseAndPlot()));  //Should allow us to set up a timer trigger.
    }
    else {
        QTimer::singleShot(samples*1000/sampleRate, this, SLOT(parseAndPlot()));
    }
}


void MainWindow::parseAndPlot()
{

    QString inByteArray;
    QString firstFiveDump;

    for (int j = 0; j < 5; j++) {
        firstFiveDump = serial.readLine();
    }

    if (samples > 4) {
        samples -= 5;
    }

    double x = 0;
    double y = 0;


    QVector<double> xValues(samples), yValues(samples);

    for (int i = 0; i<samples; i++) {
        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        xValues[i] = x;
        yValues[i] = y;
        x += 1000/double(sampleRate);
    }

    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(xValues, yValues);
    ui->customPlot->xAxis->setRange(-samples*1000/sampleRate*0.10, samples*1000/sampleRate*1.10);

    ui->customPlot->replot();
    ui->statusBar->showMessage(QString("Sampling Done!"));
    QString dead = serial.readAll();
}

void MainWindow::CVparseAndPlot()
{
    QString inByteArray;
    QString firstFiveDump;

    for (int j = 0; j < 5; j++) {
        firstFiveDump = serial.readLine();
    }

    if (samples > 4) {
        samples -= 5;
    }

    double potenApplied;
    double y = 0;

    if ((ui->toolBox2->currentIndex() == 1) ) {
        potenApplied = ui->CVstartVolt->value();
    }
    else if(ui->toolBox2->currentIndex() == 0) {
        potenApplied = ui->ASstartVolt->value();
    }

    QVector<double> xValuesUp(samples/2), yValuesUp(samples/2);
    QVector<double> xValuesDown(samples/2), yValuesDown(samples/2);


    for (int i = 0; i<samples/2; i++) {
        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        xValuesUp[i] = potenApplied;
        yValuesUp[i] = y;
        potenApplied += voltDiv;
    }
    ui->customPlot->addGraph();
    // rescale value (vertical) axis to fit the current data:        ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(0)->setData(xValuesUp, yValuesUp);
    ui->customPlot->replot();

    for (int i = 0; i<samples/2; i++) {
        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        xValuesDown[i] = potenApplied;
        yValuesDown[i] = y;
        potenApplied -= voltDiv;
    }

    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setData(xValuesDown, yValuesDown);
    ui->customPlot->xAxis->setLabel("Volts (V)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");
    ui->customPlot->xAxis->setRange(-1.024, 1.024);
    ui->customPlot->yAxis->setRange(-10, 10);
    ui->customPlot->replot();
    ui->statusBar->showMessage(QString("Sampling Done!"));
    QString dead = serial.readAll();
}

/*************************************************************************************************************/
/***************************************** CREATE MENU FUNCTIONS *********************************************/
/*************************************************************************************************************/

//-------------------------------------------------------------------------------------------------When 2000Hz Selected

void MainWindow::rate2000Selected()
{
    sampleRate = 2000;
    serial.write("changeSampleRate!2000@0#0$0%");
    ui->statusBar->showMessage(QString("Sampling Rate: 2000 Hz"));
}

//-------------------------------------------------------------------------------------------------When 5000Hz Selected

void MainWindow::rate5000Selected()
{
    sampleRate = 5000;
    serial.write("changeSampleRate!5000@0#0$0%");
    ui->statusBar->showMessage(QString("Sampling Rate: 5000 Hz"));
}

//------------------------------------------------------------------------------------------------When 10000Hz Selected

void MainWindow::rate10000Selected()
{
    sampleRate = 10000;
    serial.write("changeSampleRate!10000@0#0$0%");
    ui->statusBar->showMessage(QString("Sampling Rate: 10000 Hz"));
}

//------------------------------------------------------------------------------------------Functionality of Disconnect

void MainWindow::disconnectSelected()
{
    clearAllSelected();
    ui->statusBar->showMessage(QString("COM Port is disconnected"));
    serial.close();
}

//-----------------------------------------------------------------------------------------------Functionality of Close

void MainWindow::closeSelected()
{
    exit(0);
}

//------------------------------------------------------------------------------------------Functionality of Reset Axis

void MainWindow::resetAxis()
{

    ui->customPlot->xAxis->setRange(0, 1000*samples/sampleRate);

    if (waveNum == 2 || ui->toolBox2->currentIndex() == 1) {
        ui->customPlot->xAxis->setRange(-samples*1000/sampleRate*0.10, samples*1000/sampleRate*0.60);
    }
    else {
        ui->customPlot->xAxis->setRange(-samples*1000/sampleRate*0.10, samples*1000/sampleRate*1.10);
    }

    ui->customPlot->replot();

    //serial.close();
    //
}

//-------------------------------------------------------------------------------------------Functionality of Clear All

void MainWindow::clearAllSelected()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->replot();
}
//------------------------------------------------------------------------------------------Fill Available Serial Ports

void MainWindow::fillPortsInfo()
{
    //ui->serialPortInfoListBox->clear();
    //static const QString blankString = QObject::tr("N/A");
    //QString description;
    //QString manufacturer;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        //description = info.description();
        //manufacturer = info.manufacturer();
        list << info.portName();
        //<< (!description.isEmpty() ? description : blankString)
        //<< (!manufacturer.isEmpty() ? manufacturer : blankString)
        //<< info.systemLocation();

        //ui->serialPortInfoListBox->addItem(list.first(), list);
        for (int i = 0; i < list.size(); i++)
        {
            ui->menuSelect_Port->addAction(list.at(i));
        }

    }
}

//--------------------------------------------------------------------------------------When 10microA Resolution Chosen

void MainWindow::res10ASelected()
{
    serial.write("resolution!20@#$%");
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");

}

//----------------------------------------------------------------------------------------When 1000nA Resolution Chosen

void MainWindow::res1000nASelected()
{
    serial.write("resolution!2000@#$%");
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");
}

//-----------------------------------------------------------------------------------------When 100nA Resolution Chosen

void MainWindow::res100nASelected()
{
    serial.write("resolution!200@#$%");
}

//------------------------------------------------------------------------------------------When 10nA Resolution Chosen

void MainWindow::res10nASelected()
{
    serial.write("resolution!20@#$%");
}

/*************************************************************************************************************/
/*********************************************** DESTRUCTOR **************************************************/
/*************************************************************************************************************/

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();
}


