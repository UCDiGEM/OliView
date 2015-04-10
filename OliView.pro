#
#  OliView
#

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = OliView
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp \
         qcustomplot.cpp \
biquad.cpp \
    calibration.cpp

HEADERS  += mainwindow.h \
         qcustomplot.h \
        biquad.h \
    calibration.h

FORMS    += mainwindow.ui \
    calibration.ui

RC_ICONS = Icon.ico
    
RESOURCES += \
    OliView.qrc

