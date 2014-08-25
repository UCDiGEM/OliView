

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h" 
#include "Biquad.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    

    QTimer *timer;

private slots:
    void setupWaveTypes();
    void setupAldeSensGraph(QCustomPlot *customPlot);

    void setUpComPort();
    void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
    void waveType();
    void fillPortsInfo();
    void preParse();
    void readEverything();

    void parseAndPlot();
    void CVparseAndPlot();
    void mouseWheel();
    void mousePress();
    void removeSelectedGraph();
    void exportSelectedGraph();
    void exportAll();
    void filterSelectedGraph();

    void loadEnzymeData();
    void aboutUs();
    void qualityCorrectSpin(int newValue);
    void qualityCorrectDial(double newValue);

    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void selectionChanged();
    void sampASPressed();
    void sampPAPressed();
    void sampCVPressed();
    void resetAxis();
    void resetGraphNames();

    void clearAllSelected();
    void closeSelected();
    void disconnectSelected();
    void graphClicked(QCPAbstractPlottable *plottable);

    void res10ASelected();
    void res10nASelected();
    void res100nASelected();
    void res1000nASelected();

    void rate2000Selected();
    void rate5000Selected();
    void rate10000Selected();

    void statsCheck(int toolBoxIndex);
    void statsUpdate();
    void brushUpdate(int startPosition);

private:
    Ui::MainWindow *ui;
    QString teensyPort;
    QString exportDestination;
    QStringList everythingAvail;
    quint32 samples;

    int sampleRate;
    float gain;
    float voltDiv;
    int graphMemory;
    double timeValue;
    QElapsedTimer elapsedTimer;
    quint32 flipSample;

    int readEverything_count;
    QString readEverything_containerStart;
    QString readEverything_container;

    int waveNum;
    int count;
    int enzymeCount;

    QCPItemTracer *statsTracer;
    QCPItemBracket *selectionBracket;
    bool selectionBracketMade;


};

#endif // MAINWINDOW_H

