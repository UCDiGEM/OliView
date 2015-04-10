#include "calibration.h"
#include "ui_calibration.h"

Calibration::Calibration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Calibration)
{
    ui->setupUi(this);
}

void Calibration::resetGraphNames(){

}




Calibration::~Calibration()
{
    delete ui;
}
