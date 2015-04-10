#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QDialog>

namespace Ui {
class Calibration;
}

class Calibration : public QDialog
{
    Q_OBJECT

public:
    explicit Calibration(QWidget *parent = 0);
    ~Calibration();

private slots:
    void resetGraphNames();

private:
    Ui::Calibration *ui;
};


#endif // CALIBRATION_H
