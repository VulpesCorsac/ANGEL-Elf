#ifndef ELF_H
#define ELF_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QDebug>

#include <QVector>

#include "qcustomplot.h"

#include "../ANGEL/Angel.h"

namespace Ui {
    class Elf;
}

class Elf : public QMainWindow
{
    Q_OBJECT

private:
    LockInAmplifier *lockInAmplifier;
    Generator *generator;

    bool constructor = false;

    double minX;
    double maxX;
    double minY;
    double maxY;

    void J4F();

public:
    explicit Elf(QWidget *parent = 0);
    ~Elf();

private slots:
    void on_comboBoxRange_currentTextChanged(const QString &arg1);
    void on_graph_Clicked(QMouseEvent *event);
    void showAll();
    void on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_checkBoxAutosettingsLockInAmplifier_stateChanged(int arg1);
    void on_comboBoxTimeConstantLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_comboBoxInputVoltageRangeLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_comboBoxSensivityLockInAmplifier_currentTextChanged(const QString &arg1);

    void on_pushButtonRangeManualreplot_clicked();

    void on_comboBoxMode_currentTextChanged(const QString &arg1);

private:
    Ui::Elf *ui;
};

#endif // ELF_H
