#
#  QCustomPlot Plot Examples
#

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = OliView
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp \
         qcustomplot.cpp \
biquad.cpp

HEADERS  += mainwindow.h \
         qcustomplot.h \
        biquad.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    favicon.ico
    
RESOURCES += \
    OliView.qrc

