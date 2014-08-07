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
#include <QFile>
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
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);

    ui->customPlot->xAxis->setRange(0, 1000);
    ui->customPlot->yAxis->setRange(-10, 10);
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");

    setupAldeSensGraph(ui->customPlot);

    setupWaveTypes();

    connect(ui->sampButtonCV, SIGNAL(clicked()), this, SLOT(sampCVPressed()));
    connect(ui->sampButtonPA, SIGNAL(clicked()), this, SLOT(sampPAPressed()));
    connect(ui->sampButtonAS, SIGNAL(clicked()), this, SLOT(sampASPressed()));
    //connect(ui->reconButton, SIGNAL(clicked()), this, SLOT(reconnectButtonPressed()));
    //connect(ui->clrButton, SIGNAL(clicked()), this, SLOT(clearButtonPressed()));
    connect(ui->ASwaveType, SIGNAL(activated(int)), this, SLOT(waveType()));
    connect(ui->filterButton, SIGNAL(clicked()), this, SLOT(filterSelectedGraph()));

    connect(ui->toolBox2, SIGNAL(currentChanged(int)), this, SLOT(resetAxis()));
    connect(ui->toolBox2, SIGNAL(currentChanged(int)), this, SLOT(resetGraphNames()));
    connect(ui->freqDial, SIGNAL(sliderMoved(int)), ui->freqSpinBox, SLOT(setValue(int)));
    connect(ui->freqSpinBox, SIGNAL(valueChanged(int)), ui->freqDial, SLOT(setValue(int)));

    connect(ui->action_10_A, SIGNAL(triggered()), this, SLOT(res10ASelected()));
    connect(ui->action_10_nA, SIGNAL(triggered()), this, SLOT(res10nASelected()));
    connect(ui->action_100_nA, SIGNAL(triggered()), this, SLOT(res100nASelected()));
    connect(ui->action_1000_nA, SIGNAL(triggered()), this, SLOT(res1000nASelected()));

    connect(ui->actionClear_All, SIGNAL(triggered()), this, SLOT(clearAllSelected()));
    connect(ui->actionReset_Axis, SIGNAL(triggered()), this, SLOT(resetAxis()));
    connect(ui->actionExport_All, SIGNAL(triggered()), this, SLOT(exportAll()));

    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeSelected()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectSelected()));

    connect(ui->action2000_Hz, SIGNAL(triggered()), this, SLOT(rate2000Selected()));
    connect(ui->action5000_Hz, SIGNAL(triggered()), this, SLOT(rate5000Selected()));
    connect(ui->action10000_Hz, SIGNAL(triggered()), this, SLOT(rate10000Selected()));

    sampleRate = 2000;
    waveNum = 0;

    fillPortsInfo();
    resetAxis();

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

    ui->filterType->insertItem(0,"Low Pass");
    ui->filterType->insertItem(1,"Notch");


}


/*************************************************************************************************************/
/******************************* INITIALIZE ALL SERIAL PORT DATA FOR SAMPLING ********************************/
/*************************************************************************************************************/

//------------------------------------------------------------------------------------------Fill Available Serial Ports

void MainWindow::fillPortsInfo()
{
    //ui->serialPortInfoListBox->clear();
    //static const QString blankString = QObject::tr("N/A");
    //QString description;
    //QString manufacturer;


    QStringList list, descrip;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {


        list << info.portName();
        descrip << info.description();

        //qDebug() << descrip;

        for (int i = 0; i < descrip.size(); i++)
        {
            if (descrip.at(i).contains("teensy",Qt::CaseInsensitive)) {
                teensyPort = list.at(i);
                setUpComPort();
                return;
            }
        }
    }


    teensyPort = QInputDialog::getItem(this, "Select your COM port", "Available Ports:", list);
    setUpComPort();

}

void MainWindow::setUpComPort()
{

    serial.setPortName(teensyPort);
    if (serial.open(QIODevice::ReadWrite))
    {
        serial.setBaudRate(QSerialPort::Baud9600);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        ui->statusBar->showMessage(QString("COM Port Successfully Linked"));
        serial.write("changeSampleRate!2000@#$%^");                            //Set default Sampling Rate
        serial.write("resolution!1@#$%^");                                    //Set default Resolution
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
    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems);

    //ui->customPlot->xAxis->setAutoTickStep(false);
    //ui->customPlot->xAxis->setTickStep(10);
    ui->customPlot->axisRect()->setupFullAxesBox();


    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));
    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

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

void MainWindow::exportSelectedGraph() {

    if (ui->customPlot->selectedGraphs().size() > 0)
    {

        QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", ui->customPlot->selectedGraphs().first()->name(), "CSV files (*.csv);;Zip files (*.zip, *.7z)", 0, 0); // getting the filename (full path)
        QFile data(filename);
        if(data.open(QFile::WriteOnly |QFile::Truncate))
        {
            QTextStream output(&data);
            const QCPDataMap *dataMap = ui->customPlot->selectedGraphs().first()->data();
            QMap<double, QCPData>::const_iterator i = dataMap->constBegin();

            output << "x Value, y Value" << endl;
            while (i != dataMap->constEnd()) {
                output << i.value().key << ", " << i.value().value << endl;
                ++i;
            }
        }
    }
}

void MainWindow::exportAll() {


    for (int j = 0; j < ui->customPlot->graphCount(); ++j) {

        QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", ui->customPlot->graph(j)->name(), "CSV files (*.csv);;Zip files (*.zip, *.7z)", 0, 0); // getting the filename (full path)
        QFile data(filename);
        if(data.open(QFile::WriteOnly |QFile::Truncate))
        {
            QTextStream output(&data);
            const QCPDataMap *dataMap = ui->customPlot->graph(j)->data();
            QMap<double, QCPData>::const_iterator i = dataMap->constBegin();

            output << "x Value, y Value" << endl;
            while (i != dataMap->constEnd()) {
                output << i.value().key << ", " << i.value().value << endl;
                ++i;
            }
        }
    }
}

void MainWindow::filterSelectedGraph() {

    double frequency = double(ui->freqSpinBox->value());

    Biquad *filter;

    if(ui->customPlot->graphCount() > 0 ) {

        if (ui->filterType->currentIndex() == 0) {
            filter = new Biquad(bq_type_lowpass, frequency / sampleRate, 0.707, 0);
        }
        else if(ui->filterType->currentIndex() == 1) {
            filter = new Biquad(bq_type_notch, frequency / sampleRate, 0.707, 0);
        }

        //Biquad *filter1 = new Biquad(bq_type_lowpass, frequency / sampleRate, 0.707, 0);

        //int sampleNumber = ui->customPlot->graph(ui->graphNames->currentIndex())->keyAxis()->range().size();

        int sampleNumber = 2000;
        qDebug() << ui->graphNames->currentIndex();

        QVector<double> xValues(sampleNumber), yValues(sampleNumber);
        int counter = 0;

        const QCPDataMap *dataMap = ui->customPlot->graph(ui->graphNames->currentIndex())->data();
        QMap<double, QCPData>::const_iterator i = dataMap->constBegin();
        while (i != dataMap->constEnd()) {
            xValues[counter] = i.value().key;
            yValues[counter] = filter->process(i.value().value);
            counter++;
            ++i;
        }

        QPen pen;
        ui->customPlot->addGraph();
        int randomColorNumber = ui->customPlot->graphCount();
        pen.setColor(QColor(sin(randomColorNumber*0.3)*100+100, sin(randomColorNumber*0.6+0.7)*100+100, sin(randomColorNumber*0.4+0.6)*100+100));
        ui->customPlot->graph()->setPen(pen);
        ui->customPlot->graph()->setData(xValues, yValues);

        ui->customPlot->replot();
        ui->statusBar->showMessage(QString("Graph Filtered"));

        resetGraphNames();
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

    if (ui->customPlot->selectedGraphs().size() > 0) {
        menu->addAction("Export graph data", this, SLOT(exportSelectedGraph()));
        menu->addAction("Filter data", this, SLOT(filterSelectedGraph()));
        menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    }

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

    // synchronize selection of graphs with selection of corresponding legend items and filter box:
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

void MainWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            ui->customPlot->replot();
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

void MainWindow::sampASPressed()
{
    QString ASsv = QString::number(ui->ASstartVolt->value(), 'f', 2);
    QString ASpv = QString::number(ui->ASpeakVolt->value(), 'f', 2);
    QString ASsr = QString::number(ui->ASscanRate->value());
    QString wave = QString::number(waveNum);
    QString itr = QString::number(ui->ASiterations->value());

    QString mainInstructions = ("anoStrip!"+ASsv+"@"+ASpv+"#"+ASsr+"$"+wave+"%"+itr+"^");
    serial.write(mainInstructions.toStdString().c_str());

    qDebug() << mainInstructions;

    QTimer::singleShot(250, this, SLOT(preParse()));

    //ui->sampButton->setText(QString("Resample"));
}


//---------------------------------------------------------------------------------------------------Cyclic Voltammetry


void MainWindow::sampCVPressed()
{
    QString CVsv = QString::number(ui->CVstartVolt->value(), 'f', 2);
    QString CVpv = QString::number(ui->CVpeakVolt->value(), 'f', 2);
    QString CVsr = QString::number(ui->CVscanRate->value());

    QString mainInstructions = ("cycVolt!"+CVsv+"@"+CVpv+"#"+CVsr+"$"+"2%1^");
    serial.write(mainInstructions.toStdString().c_str());

    QTimer::singleShot(250, this, SLOT(preParse()));;

}

//-------------------------------------------------------------------------------------------Potentiostatic Amperometry

void MainWindow::sampPAPressed()
{
    QString PApv = QString::number(ui->PApotVolt->value(), 'f', 2);
    QString PAst = QString::number(ui->PAsampTime->value(), 'f', 2);

    QString mainInstructions = ("potAmpero!"+PAst+"@"+PApv+"#$%1^");
    serial.write(mainInstructions.toStdString().c_str());

    qDebug() << mainInstructions;

    QTimer::singleShot(150, this, SLOT(preParse()));

}

/*************************************************************************************************************/
/***************************** PREPARE FOR DATA TO BE READ FROM TEENSY ***************************************/
/*************************************************************************************************************/

void MainWindow::preParse() {

    this->setCursor(QCursor(Qt::BusyCursor));

    ui->statusBar->showMessage(QString("Sampling..."));

    QString sampleBuf = serial.readLine();
    QString voltDivBuf = serial.readLine();
    QString flipSampleBuf = serial.readLine();

    samples = sampleBuf.toInt();                                    //Teensy now sends the number of samples in the first line
    voltDiv = voltDivBuf.toFloat();
    flipSample = flipSampleBuf.toInt();

    qDebug() << samples;
    qDebug() << voltDiv;
    qDebug() << flipSample;

    if (ui->toolBox2->currentIndex() == 1) {
        QTimer::singleShot(samples*1000/sampleRate, this, SLOT(CVparseAndPlot()));  //Cylic Voltammetry
    }
    else if (waveNum == 2 && ui->toolBox2->currentIndex()==0) {    //Anodic Stripping, triangle wave
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(CVparseAndPlot()));
        timer->start(samples*1000/sampleRate);

    }
    else if (ui->toolBox2->currentIndex()==2) {                    // Potentiostatic Amperometry

        QTimer::singleShot(samples*1000/sampleRate, this, SLOT(parseAndPlot()));

        /*QObject::connect(&serial, SIGNAL(readyRead()), this, SLOT(readEverything()));
        ui->customPlot->addGraph();
        timeValue = 0;
        graphMemory = 0;
        ui->statusBar->showMessage(QString("Sampling..."));*/

    }
    else {

        ui->customPlot->addGraph();
        timeValue = 0;
        graphMemory = 0;

    }
}

void MainWindow::pointPlot() {

    /*everythingAvail.append(serial.readAll());

    ui->customPlot->graph()->addData(timeValue, analogRead.toDouble());
    timeValue += 1000/double(sampleRate);
    graphMemory++;
    ui->customPlot->replot();
    qDebug() << analogRead;
    qDebug() << timeValue;


    if (graphMemory >= samples) {

        qDebug() << samples;
        ui->statusBar->showMessage(QString("Sampling Done!"));
        ui->customPlot->replot();
    }*/
}

void MainWindow::readEverything() {

    everythingAvail = (QString(serial.readAll()).split("\n"));

    //qDebug() << everythingAvail.at(everythingAvail.begin());



    for(int i = 0; i < everythingAvail.length(); i++) {

        if (everythingAvail.at(everythingAvail.length()-1).size() < 9){
            readEverything_container = everythingAvail.at(i);

        }

        /*if (everythingAvail.at(i).size() < 9){
            readEverything_container = everythingAvail.at(i);

            //qDebug() << everythingAvail.at(i);
        }

        else if (everythingAvail.at(i).size() < 9) {
             timeValue += 1000/double(sampleRate);
             readEverything_containerStart = everythingAvail.at(i);
             readEverything_containerStart.append(readEverything_container);
             ui->customPlot->graph()->addData(timeValue, readEverything_containerStart.toDouble());
             graphMemory++;
             //qDebug() << everythingAvail.at(i).size();
             //qDebug() << "i :" + QString(i);
         }

        else {*/
        //qDebug() << everythingAvail.at(i).size();

        graphMemory++;
        ui->customPlot->graph()->addData(timeValue, everythingAvail.at(i).toDouble());

        timeValue += 1000/double(sampleRate);

        qDebug() << graphMemory;

        if (graphMemory >= (int)samples) {
            QObject::disconnect(&serial, SIGNAL(readyRead()), this, SLOT(readEverything()));
            everythingAvail = QStringList();

            ui->statusBar->showMessage(QString("Sampling Done!"));
            this->setCursor(QCursor(Qt::ArrowCursor));
            //}

            // Conditioning for samples that get cut-off


        }
    }
    //readEverything_count++;
    ui->customPlot->replot();
}


/*************************************************************************************************************/
/***************************** READ DATA FROM SERIAL PORT AND GRAPH THE VALUES *******************************/
/*************************************************************************************************************/
void MainWindow::parseAndPlot() {

    QString inByteArray;
    QPen pen;

    double x = 0;
    double y = 0;

    QVector<double> xValues(samples), yValues(samples);

    //qDebug() << -samples*1000/sampleRate*0.10, samples*1000/sampleRate*1.10;

    for (quint32 i = 0; i<samples; i++) {



        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        xValues[i] = x;

        yValues[i] = y;
        //LowPass Filter

        x += 1000/double(sampleRate);
    }

    ui->customPlot->addGraph();
    int randomColorNumber = ui->customPlot->graphCount()+3;
    pen.setColor(QColor(sin(randomColorNumber*0.3)*100+100, sin(randomColorNumber*0.6+0.7)*100+100, sin(randomColorNumber*0.4+0.6)*100+100));
    ui->customPlot->graph()->setPen(pen);
    ui->customPlot->graph()->setData(xValues, yValues);

    ui->customPlot->replot();

    ui->statusBar->showMessage(QString("Sampling Done!"));
    this->setCursor(QCursor(Qt::ArrowCursor));
}

/*************************************************************************************************************/
/***************************** CYCLIC VOLTAMMETRY PARSING AND PLOTTING ***************************************/
/*************************************************************************************************************/

void MainWindow::CVparseAndPlot()
{
    QString inByteArray;
    QPen pen;

    double voltMark;
    double y = 0;

    QVector<double> xValuesUp(samples), yValuesUp(samples);

    for (quint32 i = 0; i<samples; i++) {
        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        inByteArray = serial.readLine();
        voltMark = inByteArray.toDouble();
        xValuesUp[i] = voltMark;


        yValuesUp[i] = y;
    }

    int randomColorNumber = count;

    ui->customPlot->addGraph();
    //ui->customPlot->graph()->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssSquare);

    ui->customPlot->graph()->setData(xValuesUp, yValuesUp);
    pen.setColor(QColor(sin(randomColorNumber*0.3)*100+100, sin(randomColorNumber*0.6+0.7)*100+100, sin(randomColorNumber*0.4+0.6)*100+100));
    ui->customPlot->graph()->setPen(pen);
    ui->customPlot->replot();

    ui->customPlot->graph()->setLineStyle(QCPGraph::lsNone);
    //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssDisc);

    ui->customPlot->graph()->setPen(pen);
    ui->customPlot->graph()->rescaleValueAxis(true);

    ui->customPlot->replot();

    ui->statusBar->showMessage(QString("Sample #%1 done!").arg(count+1));

    count += 1;

    if (ui->toolBox2->currentIndex()==1) {
        count = 0;
        ui->statusBar->showMessage(QString("Sampling Done!"));
        this->setCursor(QCursor(Qt::ArrowCursor));
    }

    if (count >= ui->ASiterations->value() && waveNum == 2 && ui->toolBox2->currentIndex()==0) {
        count = 0;
        timer->stop();
        ui->statusBar->showMessage(QString("Sampling Done!"));
        this->setCursor(QCursor(Qt::ArrowCursor));

    }
}

/*************************************************************************************************************/
/***************************************** CREATE MENU FUNCTIONS *********************************************/
/*************************************************************************************************************/

//-------------------------------------------------------------------------------------------------When 2000Hz Selected

void MainWindow::rate2000Selected()
{
    sampleRate = 2000;
    serial.write("changeSampleRate!2000@0#0$0%^");
    ui->statusBar->showMessage(QString("Sampling Rate: 2000 Hz"));
}

//-------------------------------------------------------------------------------------------------When 5000Hz Selected

void MainWindow::rate5000Selected()
{
    sampleRate = 5000;
    serial.write("changeSampleRate!5000@0#0$0%^");
    ui->statusBar->showMessage(QString("Sampling Rate: 5000 Hz"));
}

//------------------------------------------------------------------------------------------------When 10000Hz Selected

void MainWindow::rate10000Selected()
{
    sampleRate = 10000;
    serial.write("changeSampleRate!10000@0#0$0%^");
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

    if ((ui->toolBox2->currentIndex() == 0) || ui->toolBox2->currentIndex() == 1) {
        ui->customPlot->xAxis->setLabel("Volts (V)");
        ui->customPlot->yAxis->setLabel("Microamps (µA)");
    }
    else {
        ui->customPlot->yAxis->setLabel("Microamps (µA)");
        ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    }

    ui->customPlot->replot();

    //serial.close();
}

void MainWindow::resetGraphNames()
{

    for (int j = 0; j < ui->customPlot->graphCount(); ++j) {
        ui->graphNames->insertItem(j, ui->customPlot->graph(j)->name());


    }

    ui->graphNames->setMaxCount(ui->customPlot->graphCount()+1);

    ui->customPlot->replot();

    //serial.close();

}

//-------------------------------------------------------------------------------------------Functionality of Clear All

void MainWindow::clearAllSelected()
{
    QString dead = serial.readAll();
    ui->customPlot->clearGraphs();
    ui->customPlot->replot();
    resetGraphNames();
}

//--------------------------------------------------------------------------------------When 10microA Resolution Chosen

void MainWindow::res10ASelected()
{
    serial.write("resolution!1@#$%^");

    clearAllSelected();
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");
    ui->customPlot->yAxis->setRange(-10,10);
    ui->customPlot->replot();


}

//----------------------------------------------------------------------------------------When 1000nA Resolution Chosen

void MainWindow::res1000nASelected()
{
    serial.write("resolution!2@#$%^");
    clearAllSelected();
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Nanoamps (nA)");
    ui->customPlot->yAxis->setRange(-1000,1000);
    ui->customPlot->replot();
}

//-----------------------------------------------------------------------------------------When 100nA Resolution Chosen

void MainWindow::res100nASelected()
{
    serial.write("resolution!3@#$%^");
    clearAllSelected();
    ui->customPlot->yAxis->setLabel("Nanoamps (nA)");
    ui->customPlot->yAxis->setRange(-100,100);
    ui->customPlot->replot();
}

//------------------------------------------------------------------------------------------When 10nA Resolution Chosen

void MainWindow::res10nASelected()
{
    serial.write("resolution!4@#$%^");
    clearAllSelected();
    ui->customPlot->yAxis->setLabel("Nanoamps (nA)");
    ui->customPlot->yAxis->setRange(-10,10);
    ui->customPlot->replot();
}

/*************************************************************************************************************/
/*********************************************** DESTRUCTOR **************************************************/
/*************************************************************************************************************/

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();
}


