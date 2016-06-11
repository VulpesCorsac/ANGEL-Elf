#ifndef ELF_H
#define ELF_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QDebug>

#include <QDateTime>
#include <QTime>
#include <QTimer>

#include <QDir>

#include <stdio.h>
#include <iostream>
#include <iomanip>

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

    QStringList xAxis;
    QStringList yAxis;

    QDir currentFolder;

    bool constructor = false;

    bool pause = false;
    bool stop = false;

    bool continuous = false;

    QTimer run;
    QTime allTime;

    const int waitBefore = 5000;
    const double check = false;

    int points;
    int wait;

    double from;
    double to;
    double step;

    QString reserveFileNameHeader = "Experiment_Reserve_File";
    QString userFileNameHeader = "";

    const double addUP = 0.0001;
    const double subDown = 0.001;

    // J4F
    double minX;
    double maxX;
    double minY;
    double maxY;

    void J4F();

public:
    explicit Elf(QWidget *parent = 0);
    ~Elf();

private slots:
    // Hiding and showing
    void showAll();
    void hideAll();

    // GraphPlotting
    void on_graph_Clicked(QMouseEvent *event);
    void on_comboBoxRange_currentTextChanged(const QString &arg1);
    void on_pushButtonRangeManualreplot_clicked();

    // Lock-in Amplifier
    void on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_checkBoxAutosettingsLockInAmplifier_stateChanged(int arg1);
    void on_comboBoxTimeConstantLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_comboBoxInputVoltageRangeLockInAmplifier_currentTextChanged(const QString &arg1);
    void on_comboBoxSensivityLockInAmplifier_currentTextChanged(const QString &arg1);

    // Generator
    void on_comboBoxSerialPortGenerator_currentTextChanged(const QString &arg1);

    // Experiment
    void on_comboBoxMode_currentTextChanged(const QString &arg1);

    void changeConstants();

    void on_spinBoxAverageOfPoints_valueChanged(int arg1);
    void on_spinBoxWait_valueChanged(int arg1);

    void timerStart(const int &ms = 0);
    void timerPause();
    void timerStop();

    void experimentInit();

    void on_pushButtonExport_clicked();

    void on_pushButtonStart_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonStop_clicked();


private:
    Ui::Elf *ui;
};

#endif // ELF_H
