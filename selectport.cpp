#include "selectport.h"
#include "ui_selectport.h"
#include "QSerialPort"
#include "QSerialPortInfo"
#include "mainwindow.h"
#include "QObject"
#include "QString"
#include <QIntValidator>
selectport::selectport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::selectport)
{
    setWindowTitle("Choose your comport");
    ui->setupUi(this);
    intValidator = new QIntValidator(0, 4000000, this);
    fillPortsInfo();
   // apply();
    connect(ui->serialPortListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentName()));
    connect(ui->applyButton, SIGNAL(clicked()),
            this, SLOT(apply()));
    //currentName();

   //connect(ui->applyButton, SIGNAL(clicked()),
       //     this, SLOT(apply()));

//MainWindow *mywindow = new MainWindow;
   //connect(mywindow, SIGNAL(ui->applyButton->clicked()), this, SLOT(setUpComPort()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(close()));
}

void selectport::fillPortsInfo()
{
    QStringList list;
    ui->serialPortListBox->clear();
    static const QString blankString = QObject::tr("N/A");

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        list << info.portName();}

        ui->serialPortListBox->addItems(list);
        //ui->serialPortListBox->addItem(list.first(), list);
       // qDebug() <<"list"<<list;

}
QString const selectport::currentName()
{
    name = ui->serialPortListBox->currentText();
    return name;
}
void selectport::apply()
{
    name = ui->serialPortListBox->currentText();
    qDebug()<<"name2"<<name;
    currentName();
    hide();
}

selectport::~selectport()
{
    delete ui;
}

