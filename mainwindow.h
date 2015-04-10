

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "calibration.h"
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
    void realtimePlot();

    void defaultFilter();
    void CVparseAndPlot();
    void mouseWheel();
    void mousePress();
    void removeSelectedGraph();
    void exportSelectedGraph();
    void exportAll();
    void changeExportDestination();
    void filterSelectedGraph();
    void addTracer();

    void loadEnzymeData();
    void aboutUs();
    void qualityCorrectSpin(int newValue);
    void qualityCorrectDial(double newValue);

    void contextMenuRequest(QPoint pos);
    void setGraphVisible();
    void setGraphInvisible();
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
    void calibrate();

    bool clearAreYouSure();
    void res10ASelected();
    void res10nASelected();
    void res100nASelected();
    void res1000nASelected();

    void rate2000Selected();
    void rate3000Selected();

    void statsCheck();
    void statsUpdate();
    void brushUpdate(int startPosition);

private:
    Ui::MainWindow *ui;
    Calibration *calibration;
    QString teensyPort;
    QCPItemText *resolutionText;
    QString exportDestination;
    QStringList everythingAvail;
    bool firstSampleBoolean;
    QString holdover_Start;
    QString holdover_End;
    QString realtimeString;
    bool isCV;
    Biquad *defaultFilter1;
    Biquad *defaultFilter2;
    Biquad *defaultFilter3;
    Biquad *defaultFilter4;
    QString saver;
    quint32 samples;
    int mainResolution;

    int sampleRate;
    quint32 flipSample;
    float gain;
    float voltDiv;
    float voltMark;
    quint32 graphMemory;
    double timeValue;

    QVector<double> yValuesFilter;
    QVector<double> xValues;
    QElapsedTimer elapsedTimer;


    //Used in realtime data plotting
    int readEverything_count;
    QString readEverything_containerStart;
    QString readEverything_container;

    int waveNum;
    int count;

    int globalColorCount;
    int enzymeCount;

    //Used in statistics package animations
    QCPItemBracket *selectionBracket;
    QCPItemBracket *steadyStateBracket;
    float steadyStateValue;
    float steadyStateLength;
    QPen steadyStateBracketPen;
    bool selectionBracketMade;
    bool steadyStateBracketMade;
    QPen selectionBracketPen;
};

#endif // MAINWINDOW_H

