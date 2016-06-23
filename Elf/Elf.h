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

    bool generatorActive = false;

    QTimer run;
    QTime allTime;

    double startTime;

    const int waitBefore    = 1000;
    const int updateTime    = 100;
    const int invalidPoints = 3;

    const double check = false;

    const int generator_send = 3;

    int points = 0;
    int wait   = 0;

    double from = 0;
    double to   = 0;
    double step = 0;

    QString reserveFileNameHeader = "Experiment_Reserve_File";
    QString userFileNameHeader    = "";

    const double addUp   = 0.0001;
    const double subDown = 0.0001;

    SimpleExperimentData experimentData;

public:
    explicit Elf(QWidget *parent = 0);
    ~Elf();

    void stopAll();

private slots:
    bool inRange(const double &min, const double &max, const double &value);

    // Hiding and showing
    void showAll();
    void hideAll();

    // GraphPlotting
    void on_graph_Clicked(QMouseEvent *event);
    void on_comboBoxRange_currentTextChanged(const QString &arg1);
    void on_pushButtonRangeManualreplot_clicked();

    void updateGraph();
    void replotGraph();
    void pushGraph(const SimpleExperimentPoint &point);

    void on_comboBoxXAxisValue_currentTextChanged(const QString &arg1);
    void on_comboBoxYAxisValue_currentTextChanged(const QString &arg1);

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

    QString getFileName(const QString& header = "");

    void on_pushButtonExport_clicked();

    void on_pushButtonStart_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonStop_clicked();

    void experiment_StartingPoint();
    void experiment_Run();

private:
    Ui::Elf *ui;
};

#endif // ELF_H
