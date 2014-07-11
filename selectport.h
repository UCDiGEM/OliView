#ifndef SELECTPORT_H
#define SELECTPORT_H

#include <QDialog>
#include <QMainWindow>
//#include<QSerialPort>
#include <QIntValidator>
#include <QtSerialPort/QSerialPort>
namespace Ui {
class selectport;
}


class selectport : public QDialog
{
    Q_OBJECT

public:
  QString name;
 // QSerialPort serial1;

    explicit selectport(QWidget *parent = 0);
    ~selectport();

     QString const currentName();

private:
    Ui::selectport *ui;
    QIntValidator *intValidator;


private slots:
    //void setUpComPort();
    void fillPortsInfo();
   // void updateSettings();
    void apply();

//signals:
//  void choosePort(QString portName);
};

#endif // SELECTPORT_H
