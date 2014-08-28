/************************************************************************************************************
**                                                                                                         **
**  This was developed from example code for QCustomPlot                                                   **
**  by UC Davis iGEM 2014                                                                                  **
**  Coding by E. Aaron Cohen, Michaela Gobron, Julie Yeonju Song                                           **
**                                                                                                         **
*************************************************************************************************************/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>
#include <QFile>
#include <QMenuBar>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtAlgorithms>

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
    ui->customPlot->yAxis->setRange(-100, 100);
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");

    setupAldeSensGraph(ui->customPlot);

    setupWaveTypes();

    connect(ui->sampButtonCV, SIGNAL(clicked()), this, SLOT(sampCVPressed()));
    connect(ui->sampButtonPA, SIGNAL(clicked()), this, SLOT(sampPAPressed()));
    connect(ui->showRawData, SIGNAL(clicked()), this, SLOT(showRawData()));
    connect(ui->sampButtonAS, SIGNAL(clicked()), this, SLOT(sampASPressed()));
    //connect(ui->reconButton, SIGNAL(clicked()), this, SLOT(reconnectButtonPressed()));
    //connect(ui->clrButton, SIGNAL(clicked()), this, SLOT(clearButtonPressed()));
    connect(ui->ASwaveType, SIGNAL(activated(int)), this, SLOT(waveType()));
    connect(ui->filterButton, SIGNAL(clicked()), this, SLOT(filterSelectedGraph()));

    connect(ui->toolBox2, SIGNAL(currentChanged(int)), this, SLOT(resetAxis()));
    connect(ui->toolBox2, SIGNAL(currentChanged(int)), this, SLOT(resetGraphNames()));
    connect(ui->toolBox2, SIGNAL(currentChanged(int)), this, SLOT(statsCheck()));
    connect(ui->freqDial, SIGNAL(sliderMoved(int)), ui->freqSpinBox, SLOT(setValue(int)));
    connect(ui->freqSpinBox, SIGNAL(valueChanged(int)), ui->freqDial, SLOT(setValue(int)));

    connect(ui->qualDial, SIGNAL(sliderMoved(int)), this, SLOT(qualityCorrectSpin(int)));
    connect(ui->qualSpinBox, SIGNAL(valueChanged(double)), this, SLOT(qualityCorrectDial(double)));

    connect(ui->loadEnzymeData, SIGNAL(clicked()), this, SLOT(loadEnzymeData()));

    connect(ui->startSlider, SIGNAL(sliderReleased()), this, SLOT(statsUpdate()));
    //connect(ui->graphNames_2, SIGNAL(currentIndexChanged(int)), this, SLOT(statsUpdate()));
    connect(ui->startSlider, SIGNAL(sliderMoved(int)), this, SLOT(brushUpdate(int)));

    connect(ui->action_10_A, SIGNAL(triggered()), this, SLOT(res10ASelected()));
    connect(ui->action_10_nA, SIGNAL(triggered()), this, SLOT(res10nASelected()));
    connect(ui->action_100_nA, SIGNAL(triggered()), this, SLOT(res100nASelected()));
    connect(ui->action_1000_nA, SIGNAL(triggered()), this, SLOT(res1000nASelected()));

    connect(ui->actionClearAll, SIGNAL(triggered()), this, SLOT(clearAllSelected()));
    connect(ui->actionReset_Axis, SIGNAL(triggered()), this, SLOT(resetAxis()));
    connect(ui->actionExport_All, SIGNAL(triggered()), this, SLOT(exportAll()));
    connect(ui->actionChangeExportDestination, SIGNAL(triggered()), this, SLOT(changeExportDestination()));

    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeSelected()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectSelected()));

    connect(ui->action2000_Hz, SIGNAL(triggered()), this, SLOT(rate2000Selected()));
    connect(ui->action5000_Hz, SIGNAL(triggered()), this, SLOT(rate5000Selected()));

    connect(ui->actionAbout_Us, SIGNAL(triggered()), this, SLOT(aboutUs()));

    sampleRate = 2000;
    waveNum = 0;
    selectionBracketMade = false;
    steadyStateBracketMade = false;
    steadyStateValue = 0.0002;
    steadyStateLength = 5000;

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

    ui->ASwaveType->insertItem(0, sineWave,(const char *) 0);
    ui->ASwaveType->setIconSize(QSize(100,28));
    ui->ASwaveType->insertItem(1, triangleWave,(const char *) 0);
    ui->ASwaveType->setIconSize(QSize(100,28));
    ui->ASwaveType->setCurrentIndex(0);


}

/*void MainWindow::addEnzyme() {
    enzymeCount++;
    ui->enzymeList->setRowCount(1+enzymeCount);
}

void MainWindow::addSubstrate() {

}*/

void MainWindow::loadEnzymeData() {
    QString filename = QFileDialog::getOpenFileName(this, "DialogTitle", "EnzymeData.csv", "CSV files (*.csv);;Zip files (*.zip, *.7z)", 0, 0);
    QFile data(filename);

    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream input(&data);
        /*qDebug() << input;
        int cnt1 = 0;  int cnt2 = 0;
        while (i != input->constEnd()) {
            cnt1++;  cnt2++;
            ui->enzymeList->setItem(1,1, new QTableWidgetItem(text));
        }*/
    }


}

void MainWindow::aboutUs() {

}

void MainWindow::qualityCorrectDial(double newValue) {
    int newValueInt = qRound(1000.0*log((1.0+newValue)));
    //qDebug() << newValueInt;
    ui->qualDial->setValue(newValueInt);
}

void MainWindow::qualityCorrectSpin(int newValue) {
    double newValueDouble = qExp(newValue/1000.0)-1.0;
    //qDebug() << newValueDouble;
    ui->qualSpinBox->setValue(newValueDouble);

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

    ui->filterType->insertItem(0,"Low Pass");
    ui->filterType->insertItem(1,"Notch");

    ui->enzymeList->setSortingEnabled(false);
    ui->enzymeList->setRowCount(3);
    ui->enzymeList->setColumnCount(3);

    ui->enzymeListB->setSortingEnabled(false);
    ui->enzymeListB->setRowCount(3);
    ui->enzymeListB->setColumnCount(3);

    ui->enzymeListC->setSortingEnabled(false);
    ui->enzymeListC->setRowCount(3);
    ui->enzymeListC->setColumnCount(3);

    QStringList headers;
    headers << "Substrate" << "Km" << "Kcat";
    ui->enzymeList->setHorizontalHeaderLabels(headers);
    ui->enzymeList->setToolTip("Km (uM), Kcat (nmol/min/mg)");

    ui->enzymeListB->setHorizontalHeaderLabels(headers);
    ui->enzymeListB->setToolTip("Km (uM), Kcat (nmol/min/mg)");

    ui->enzymeListC->setHorizontalHeaderLabels(headers);
    ui->enzymeListC->setToolTip("Km (uM), Vmax (nmol/min/mg)");

    QString style = "::section{"
            "padding-right: 3px;"
            "border: 2px; }";

    ui->enzymeList->horizontalHeader()->setStyleSheet(style);
    ui->enzymeListB->horizontalHeader()->setStyleSheet(style);
    ui->enzymeListC->horizontalHeader()->setStyleSheet(style);

    ui->enzymeList->setColumnWidth(0,75);
    ui->enzymeList->setColumnWidth(1,65);
    ui->enzymeList->setColumnWidth(2,65);

    ui->enzymeListB->setColumnWidth(0,75);
    ui->enzymeListB->setColumnWidth(1,65);
    ui->enzymeListB->setColumnWidth(2,65);

    ui->enzymeListC->setColumnWidth(0,75);
    ui->enzymeListC->setColumnWidth(1,65);
    ui->enzymeListC->setColumnWidth(2,65);

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
        // if a bracket exists and belongs to the graph being deleted..
        if (selectionBracketMade && (ui->customPlot->selectedGraphs().first() == ui->customPlot->graph(ui->graphNames_2->currentIndex()))) {
            // remove the bracket before removing graph
            ui->customPlot->removeItem(selectionBracket);
            selectionBracketMade = false;
        }

        //remove graph selected by context menu
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
        ui->customPlot->replot();
        //reset graph name selectors in toolbox
        resetGraphNames();


        // Recheck the stats incase active graph was removed
        statsCheck();

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

void MainWindow::changeExportDestination() {
    exportDestination = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/home",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

void MainWindow::exportAll() {
    for (int j = 0; j < ui->customPlot->graphCount(); ++j) {

        QFile data(exportDestination+"/"+ui->customPlot->graph(j)->name()+".csv");
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

        int counter = 0;

        const QCPDataMap *dataMap = ui->customPlot->graph(ui->graphNames->currentIndex())->data();
        QMap<double, QCPData>::const_iterator i = dataMap->constBegin();

        int sampleNumber = dataMap->count();
        QVector<double> xValues(sampleNumber), yValues(sampleNumber);

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
        statsCheck();
    }
}

//----------------------------------------------------------------------------------------------------Remove All Graphs

void MainWindow::removeAllGraphs()
{
    resetGraphNames();
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
        menu->addAction("Set graph visible", this, SLOT(setGraphVisible()));
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

void MainWindow::setGraphVisible()
{
    for(int i = 0; i < ui->customPlot->graphCount(); i++) {
        if(ui->customPlot->graph(i)->selected()) {
            ui->customPlot->graph(i)->setVisible(true);
            ui->customPlot->replot();
        }
    }
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

    QTimer::singleShot(250, this, SLOT(preParse()));

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


void MainWindow::readEverything() {

    everythingAvail = (QString(serial.readAll()).split("\n"));

    //qDebug() << everythingAvail.at(everythingAvail.begin());



    for(int i = 0; i < everythingAvail.length(); i++) {

        //if (everythingAvail.at(0).size() < 9){
        //  everythingAvail.at(0).append(readEverything_container = );

        //}

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
        qDebug() << everythingAvail.at(i);
        qDebug() << everythingAvail.at(i).size();

        graphMemory++;
        ui->customPlot->graph()->addData(timeValue, everythingAvail.at(i).toDouble());

        timeValue += 1000/double(sampleRate);

        //qDebug() << graphMemory;

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



/*void MainWindow::readEverything() {

    // Read everything is called by readyRead to read all incoming data
    everythingAvail = (QString(serial.readAll()).split("\n"));

    //qDebug() << everythingAvail.at(everythingAvail.begin());



    for(int i = 0; i < everythingAvail.length(); i++) {

        if (everythingAvail.at(everythingAvail.length()-1).size() < 9){
            readEverything_containerStart = everythingAvail.at(everythingAvail.length()-1);

        }

        if (everythingAvail.at(0).size() < 9){
            readEverything_container = readEverything_containerStart.append(everythingAvail.at(0));
            timeValue += 1000/double(sampleRate);
            ui->customPlot->graph()->addData(timeValue, readEverything_container.toDouble());
            graphMemory++;

            //qDebug() << everythingAvail.at(i);
        }

        else {
            //qDebug() << everythingAvail.at(i).size();

            graphMemory++;
            ui->customPlot->graph()->addData(timeValue, everythingAvail.at(i).toDouble());

            timeValue += 1000/double(sampleRate);

            //qDebug() << graphMemory;

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
}*/


/*************************************************************************************************************/
/***************************** READ DATA FROM SERIAL PORT AND GRAPH THE VALUES *******************************/
/*************************************************************************************************************/
void MainWindow::parseAndPlot() {

    Biquad *filter = new Biquad(bq_type_lowpass, 25.0 / sampleRate, 0.707, 0);

    //qDebug() << (sampleRate/100.0) / sampleRate;

    Biquad *filter1 = new Biquad(bq_type_notch, 60.0 / sampleRate, 0.707, 0);
    Biquad *filter2 = new Biquad(bq_type_notch, 120.0 / sampleRate, 0.707, 0);
    Biquad *filter3 = new Biquad(bq_type_notch, 180.0 / sampleRate, 0.707, 0);


    QString inByteArray;
    QPen pen;

    double x = 0;
    double y = 0;

    QVector<double> xValues(samples), yValuesFilter(samples), yValues(samples);

    //qDebug() << -samples*1000/sampleRate*0.10, samples*1000/sampleRate*1.10;

    for (quint32 i = 0; i<samples; i++) {



        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        xValues[i] = x;

        yValuesFilter[i] = filter3->process(filter2->process(filter1->process(filter->process(y))));
        yValues[i] = y;
        //LowPass Filter

        x += 1000/double(sampleRate);
    }

    ui->customPlot->addGraph();
    int randomColorNumber = ui->customPlot->graphCount()+3;
    pen.setColor(QColor(sin(randomColorNumber*0.3)*100+100, sin(randomColorNumber*0.6+0.7)*100+100, sin(randomColorNumber*0.4+0.6)*100+100));
    ui->customPlot->graph()->setPen(pen);
    ui->customPlot->graph()->setData(xValues, yValuesFilter);

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setPen(pen);
    ui->customPlot->graph()->setData(xValues, yValues);
    ui->customPlot->graph()->setVisible(false);



    ui->customPlot->replot();

    ui->statusBar->showMessage(QString("Sampling Done!"));
    this->setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::showRawData() {
    ui->customPlot->graph()->setVisible(true);
    ui->customPlot->replot();
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


    if ((ui->toolBox2->currentIndex() == 1) ) {
        ui->customPlot->xAxis->setRange(ui->CVstartVolt->value()*1.1, ui->CVpeakVolt->value()*1.1);
    }
    else if(ui->toolBox2->currentIndex() == 0) {
        ui->customPlot->xAxis->setRange(ui->ASstartVolt->value()*1.1, ui->ASpeakVolt->value()*1.1);
    }


    ui->customPlot->replot();
    QVector<double> xValuesUp(samples), yValuesUp(samples);

    for (quint32 i = 0; i<samples; i++) {
        inByteArray = serial.readLine();
        y = inByteArray.toDouble();
        inByteArray = serial.readLine();
        voltMark = inByteArray.toDouble();
        xValuesUp[i] = voltMark;


        yValuesUp[i] = y;
    }

    int randomColorNumber = ui->customPlot->graphCount()+3;

    ui->customPlot->addGraph();

    ui->customPlot->graph()->setData(xValuesUp, yValuesUp);

    QCPScatterStyle scatterVolt;


    scatterVolt.setPen(QColor(sin(randomColorNumber*0.3)*100+100, sin(randomColorNumber*0.6+0.7)*100+100, sin(randomColorNumber*0.4+0.6)*100+100));
    scatterVolt.setShape(QCPScatterStyle::ssCircle);
    scatterVolt.setSize(0.5);
    ui->customPlot->graph()->setScatterStyle(scatterVolt);
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
/*************************************** STATISTICS PACKAGE **************************************************/
/*************************************************************************************************************/


void MainWindow::statsCheck() {
    if(ui->toolBox2->currentIndex() !=4) {
        if (selectionBracketMade) {
            selectionBracket->setVisible(false);
            ui->customPlot->replot();
        }
        else {
            return;
        }
    }

    else {
        if (ui->customPlot->graphCount() > 0 && !selectionBracketMade) {
            const QCPDataMap *dataMap = ui->customPlot->graph(ui->graphNames_2->currentIndex())->data();
            int sampleNumber = dataMap->count();
            int counter = 0;
            QVector<double> xValues(sampleNumber), yValues(sampleNumber);

            QMap<double, QCPData>::const_iterator j = dataMap->constBegin();
            while (j != dataMap->constEnd()) {
                xValues[counter] = j.value().key;
                yValues[counter] = j.value().value;
                counter++;
                ++j;
            }

            qSort(yValues.begin(), yValues.end());
            float max = yValues[sampleNumber-1];

            QCPItemBracket *bracket = new QCPItemBracket(ui->customPlot);
            selectionBracket = bracket;
            selectionBracket->setStyle(QCPItemBracket::bsSquare);

            selectionBracketPen.setWidthF(1.5);
            selectionBracket->setPen(selectionBracketPen);
            ui->customPlot->addItem(selectionBracket);
            selectionBracketMade = true;
            selectionBracket->left->setCoords(0, max*1.1);
            selectionBracket->right->setCoords(dataMap->count()/(sampleRate/1000), max*1.1);
            ui->customPlot->replot();


            int steadyStateMilliSecs = sampleRate/10;
            float tempMeanX = 0;
            float tempMeanY = 0;
            float steadyStateSumNumer = 0;
            float steadyStateSumDenom = 0;
            float estimatedSlope;
            if (sampleNumber > steadyStateMilliSecs) {

                //Moving average linear regression to determine if steady state has been reached
                QMap<double, QCPData>::const_iterator k = dataMap->constBegin();
                while (k != dataMap->constEnd()) {
                    QVector<double> steadyStateX(steadyStateMilliSecs), steadyStateY(steadyStateMilliSecs);
                    for (int i = 0; i < steadyStateMilliSecs; i++){
                        tempMeanX += k.value().key;
                        tempMeanY += k.value().value;
                    }

                    float meanX = tempMeanX/steadyStateMilliSecs;
                    float meanY = tempMeanY/steadyStateMilliSecs;


                    for (int i = 0; i < steadyStateMilliSecs; i++){
                        steadyStateX[i] = k.value().key;
                        //qDebug() << k.value().key;
                        steadyStateY[i] = k.value().value;
                        steadyStateSumNumer += (k.value().key - meanX)*(k.value().value-meanY);
                        steadyStateSumDenom += qPow((k.value().key-meanX),2);
                        ++k;
                    }

                    estimatedSlope = steadyStateSumNumer/steadyStateSumDenom;
                    qSort(steadyStateY.begin(), steadyStateY.end());
                    float localMin = yValues[0];

                    if(steadyStateBracketMade) {
                        return;
                    }
                    //This is the steady state assumption controller
                    //if the linear regression estimate of the slope is smaller than the user defined limit,
                    //and there is 5 seconds of sample data following, steady state has been reached
                    if (qAbs(estimatedSlope) <= steadyStateValue && (1000*sampleNumber/sampleRate - k.value().key) >= steadyStateLength) {
                        QCPItemBracket *steadyBracket = new QCPItemBracket(ui->customPlot);
                        steadyStateBracket = steadyBracket;
                        steadyStateBracket->setStyle(QCPItemBracket::bsSquare);

                        //build bracket above steady state
                        steadyStateBracketPen.setWidthF(1.5);
                        steadyStateBracketPen.setColor(QColor(168,182,120));
                        steadyStateBracket->setPen(steadyStateBracketPen);
                        ui->customPlot->addItem(steadyStateBracket);
                        steadyStateBracket->left->setCoords(k.value().key+steadyStateLength, localMin);
                        steadyStateBracket->right->setCoords(k.value().key, localMin);

                        float steadyStateSum;
                        for (int i = 0; i < steadyStateLength*sampleRate/1000; i++){
                            steadyStateSum += k.value().value;
                            ++k;
                        }

                        float stdyStateMean = steadyStateSum/steadyStateLength*sampleRate/1000;
                        ui->steadyStateMeanLabel->setText(QString("%1").arg(stdyStateMean));

                        ui->customPlot->replot();
                        ui->statusBar->showMessage(QString("Steady State Reached!"));
                        steadyStateBracketMade = true;
                        return;

                    }
                    qDebug() << QString("Slope Estimate:%1     time:%2 ").arg(estimatedSlope).arg(k.value().key);
                    steadyStateX.clear();
                    steadyStateY.clear();
                    steadyStateSumNumer = 0;
                    steadyStateSumDenom = 0;
                    //k-steadyStateMilliSecs;
                }
            }





        }
        if (selectionBracketMade) {
            selectionBracket->setVisible(true);
            ui->customPlot->replot();
        }
    }
    statsUpdate();

}

void MainWindow::statsUpdate()
{
    if(ui->customPlot->graphCount() > 0) {
        float meanAddition = 0;

        const QCPDataMap *dataMap = ui->customPlot->graph(ui->graphNames_2->currentIndex())->data();
        QMap<double, QCPData>::const_iterator i = dataMap->constBegin();
        ui->startSlider->setMaximum(dataMap->count()/(sampleRate/1000)-1);

        int counter = 0;
        while (i != dataMap->constEnd()) {
            ++i;
            if (i.value().key >= ui->startSlider->value()){
                meanAddition += i.value().value;
                counter++;
            }
        }

        float mean = meanAddition/counter;
        float varianceAddition = 0;
        counter = 0;

        int sampleNumber = dataMap->count()-ui->startSlider->value()*(sampleRate/1000);
        //qDebug() << sampleNumber;
        QVector<double> xValues(sampleNumber), yValues(sampleNumber);

        QMap<double, QCPData>::const_iterator j = dataMap->constBegin();
        while (j != dataMap->constEnd()) {
            if (j.value().key >= ui->startSlider->value()){
                xValues[counter] = j.value().key;
                yValues[counter] = j.value().value;
                varianceAddition += qPow((j.value().value-mean),2);
                counter++;
            }
            ++j;
        }


        qSort(yValues.begin(), yValues.end());
        float min = yValues[0];
        //qDebug() << yValues[0];
        float max = yValues[sampleNumber-1];

        float variance = varianceAddition/counter;
        float sDeviation = qSqrt(variance);

        ui->minLabel->setText(QString("%1").arg(min));
        ui->maxLabel->setText(QString("%1").arg(max));
        ui->meanLabel->setText(QString("%1").arg(mean));
        ui->sDeviationLabel->setText(QString("+/-  %1").arg(sDeviation));

        brushUpdate(ui->startSlider->value());
        //ui->statusBar->showMessage(QString("%1").arg(mean));

    }
}


void MainWindow::brushUpdate(int startPosition) {
    if(ui->customPlot->graphCount() > 0 && ui->toolBox2->currentIndex()==4) {

        const QCPDataMap *dataMap = ui->customPlot->graph(ui->graphNames_2->currentIndex())->data();
        int sampleNumber = dataMap->count();
        int counter = 0;
        QVector<double> xValues(sampleNumber), yValues(sampleNumber);

        QMap<double, QCPData>::const_iterator j = dataMap->constBegin();
        while (j != dataMap->constEnd()) {
            xValues[counter] = j.value().key;
            yValues[counter] = j.value().value;
            counter++;
            ++j;
        }

        qSort(yValues.begin(), yValues.end());
        float max = yValues[sampleNumber-1];
        selectionBracket->left->setCoords(startPosition, max*1.10);
        selectionBracket->right->setCoords(dataMap->count()/(sampleRate/1000), max*1.10);

        ui->customPlot->replot();
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
    }
    else {
        ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    }

    ui->customPlot->replot();

    //serial.close();
}

void MainWindow::resetGraphNames()
{
    //This function will replace the two graph name selectors in the Filtering & Statistics tabs with updated graph names.

    ui->graphNames->clear();
    ui->graphNames_2->clear();

    for (int j = 0; j < ui->customPlot->graphCount(); j++) {
        ui->graphNames->insertItem(j, ui->customPlot->graph(j)->name());
        ui->graphNames_2->insertItem(j, ui->customPlot->graph(j)->name());
    }



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

    if (selectionBracketMade) {
        ui->customPlot->removeItem(selectionBracket);
        selectionBracketMade = false;

    }
    if (steadyStateBracketMade) {
        ui->customPlot->removeItem(steadyStateBracket);
        steadyStateBracketMade = false;
    }

}

//--------------------------------------------------------------------------------------When 10microA Resolution Chosen

void MainWindow::res10ASelected()
{
    serial.write("resolution!1@#$%^");

    clearAllSelected();
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");
    ui->customPlot->yAxis->setRange(-100,100);
    ui->customPlot->replot();


}

//----------------------------------------------------------------------------------------When 1000nA Resolution Chosen

void MainWindow::res1000nASelected()
{
    serial.write("resolution!2@#$%^");
    clearAllSelected();
    ui->customPlot->xAxis->setLabel("Milliseconds (ms)");
    ui->customPlot->yAxis->setLabel("Microamps (µA)");
    ui->customPlot->yAxis->setRange(-10,10);
    ui->customPlot->replot();
}

//-----------------------------------------------------------------------------------------When 100nA Resolution Chosen

void MainWindow::res100nASelected()
{
    serial.write("resolution!3@#$%^");
    clearAllSelected();
    ui->customPlot->yAxis->setLabel("Nanoamps (nA)");
    ui->customPlot->yAxis->setRange(-1000,1000);
    ui->customPlot->replot();
}

//------------------------------------------------------------------------------------------When 10nA Resolution Chosen

void MainWindow::res10nASelected()
{
    serial.write("resolution!4@#$%^");
    clearAllSelected();
    ui->customPlot->yAxis->setLabel("Nanoamps (nA)");
    ui->customPlot->yAxis->setRange(-100,100);
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


