#include "Elf.h"
#include "ui_Elf.h"

Elf::Elf(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Elf)
{
    qDebug() << "Elf constructor started";

    this->constructor = true;

    ui->setupUi(this);

    currentFolder = QDir();
    QDir dataFolder = QDir(currentFolder.absolutePath() + "\\Data");
    if (!dataFolder.exists())
        dataFolder.mkpath(".");
    this->userFileNameHeader = currentFolder.absolutePath() + "\\Data\\";

    QDir reserveDataFolder = QDir(currentFolder.absolutePath() + "\\Data\\Reserve");
    if (!reserveDataFolder.exists())
        reserveDataFolder.mkpath(".");
    this->reserveFileNameHeader = currentFolder.absolutePath() + "\\Data\\Reserve\\" + this->reserveFileNameHeader;

    this->lockInAmplifier = new LockInAmplifier();
    this->generator = new Generator();

    this->xAxis.push_back("Fext");
    this->xAxis.push_back("Time");
    ui->comboBoxXAxisValue->clear();
    ui->comboBoxXAxisValue->addItems(this->xAxis);

    this->yAxis.push_back("R");
    this->yAxis.push_back("Theta");
    ui->comboBoxYAxisValue->clear();
    ui->comboBoxYAxisValue->addItems(this->yAxis);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBoxSerialPortLockInAmplifier->addItem(info.portName());
        ui->comboBoxSerialPortGenerator->addItem(info.portName());
    }

    ui->comboBoxSerialPortLockInAmplifier->setCurrentText("COM3");
    ui->comboBoxSerialPortGenerator->setCurrentText("COM3");

    ui->comboBoxMode->addItem("Single");
    ui->comboBoxMode->addItem("Continuous");

    ui->comboBoxRange->addItem("Auto");
    ui->comboBoxRange->addItem("Manual");

    //*
    ui->labelTimeConstantLockInAmplifier->hide();
    ui->comboBoxTimeConstantLockInAmplifier->hide();
    ui->checkBoxAutosettingsLockInAmplifier->hide();
    ui->labelSensivityLockInAmplifier->hide();
    ui->comboBoxSensivityLockInAmplifier->hide();
    ui->labelInputRangeLockInAmplifier->hide();
    ui->comboBoxInputVoltageRangeLockInAmplifier->hide();

    ui->labelAmplitudeGenerator->hide();
    ui->doubleSpinBoxAmplitudeGenerator->hide();
    ui->labelOffsetGenerator->hide();
    ui->doubleSpinBoxOffsetGenerator->hide();
    ui->labelFrequencyFromGenerator->hide();
    ui->doubleSpinBoxFrequencyFromGenerator->hide();
    ui->labelFrequencyToGenerator->hide();
    ui->doubleSpinBoxFrequencyToGenerator->hide();
    ui->labelFrequencyStepGenerator->hide();
    ui->doubleSpinBoxFrequencyStepGenerator->hide();

    hideAll();
    //*/

    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);

    ui->pushButtonExport->setEnabled(false);

    ui->progressBarExperiment->setValue(0);
    ui->lcdNumber->display(0);

    //* OPTIMIZE FOR SPEED
    ui->customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->customPlot->xAxis->setTickLabelFont(font);
    ui->customPlot->yAxis->setTickLabelFont(font);
    ui->customPlot->legend->setFont(font);
    //*/

    ui->customPlot->clearGraphs();
    ui->customPlot->addGraph();

    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
    ui->customPlot->axisRect()->setupFullAxesBox();

    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(on_graph_Clicked(QMouseEvent*)));

    J4F();

    this->constructor = false;

    qDebug() << "Elf constructor finished";
}

Elf::~Elf()
{
    qDebug() << "Elf destructor started";

    this->generator->disconnect();
    this->generator->~Generator();

    this->lockInAmplifier->disconnect();
    this->lockInAmplifier->~LockInAmplifier();

    delete ui;

    qDebug() << "Elf destructor finished";
}

void Elf::J4F()
{
    QVector <double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(-1, 1);
    ui->customPlot->yAxis->setRange(0, 1);
    ui->customPlot->replot();

    this->maxX = this->maxY = 1;
    this->minX = -1;
    this->minY = 0;

    ui->lineEditRangeXmin->setText(QString::number(minX));
    ui->lineEditRangeXmax->setText(QString::number(maxX));
    ui->lineEditRangeYmin->setText(QString::number(minY));
    ui->lineEditRangeYmax->setText(QString::number(maxY));
}

// Hiding and showing

void Elf::showAll() {
    qDebug() << "Showing all";

    ui->groupBoxExpriment->show();
    ui->groupBoxGraph->show();
    ui->groupBoxTiming->show();
    ui->groupBoxCurrentReadings->show();

    ui->customPlot->show();

    return;
}

void Elf::hideAll() {
    qDebug() << "Hiding all";

    ui->groupBoxExpriment->hide();
    ui->groupBoxGraph->hide();
    ui->groupBoxTiming->hide();
    ui->groupBoxCurrentReadings->hide();

    ui->customPlot->hide();

    return;
}

// GraphPlotting

void Elf::on_graph_Clicked(QMouseEvent *event)
{
    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    ui->lineEditPointX->setText(QString::number(x));
    ui->lineEditPointY->setText(QString::number(y));

    return;
}

void Elf::on_comboBoxRange_currentTextChanged(const QString &arg1)
{
    qDebug() << "Graph range mode was changed to" << arg1;

    if (arg1 == "Auto") {
        ui->pushButtonRangeManualreplot->hide();

        ui->labelRangeXmin->hide();
        ui->lineEditRangeXmin->hide();
        ui->labelRangeXmax->hide();
        ui->lineEditRangeXmax->hide();
        ui->labelRangeYmin->hide();
        ui->lineEditRangeYmin->hide();
        ui->labelRangeYmax->hide();
        ui->lineEditRangeYmax->hide();

        ui->lineEditRangeXmax->setText(QString::number(maxX));
        ui->lineEditRangeXmin->setText(QString::number(minX));
        ui->lineEditRangeYmax->setText(QString::number(maxY));
        ui->lineEditRangeYmin->setText(QString::number(minY));

        on_pushButtonRangeManualreplot_clicked();
    } else {
        ui->pushButtonRangeManualreplot->show();

        ui->labelRangeXmin->show();
        ui->lineEditRangeXmin->show();
        ui->labelRangeXmax->show();
        ui->lineEditRangeXmax->show();
        ui->labelRangeYmin->show();
        ui->lineEditRangeYmin->show();
        ui->labelRangeYmax->show();
        ui->lineEditRangeYmax->show();
    }

    return;
}

void Elf::on_pushButtonRangeManualreplot_clicked()
{
    qDebug() << "Manual replot asked for";

    ui->customPlot->xAxis->setRange(ui->lineEditRangeXmin->text().toDouble(),
                                    ui->lineEditRangeXmax->text().toDouble());
    ui->customPlot->yAxis->setRange(ui->lineEditRangeYmin->text().toDouble(),
                                    ui->lineEditRangeYmax->text().toDouble());
    ui->customPlot->replot();

    return;
}

// Lock-in Amplifier

void Elf::on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1)
{
    if (this->constructor)
        return;

    if (this->lockInAmplifier->isActive()) {
        this->lockInAmplifier->disconnect();

        ui->labelSerialPortLockInAmplifier->setText("Disconnected");

        ui->labelTimeConstantLockInAmplifier->hide();
        ui->comboBoxTimeConstantLockInAmplifier->hide();

        ui->checkBoxAutosettingsLockInAmplifier->hide();

        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();

        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();

        hideAll();

        return;
    }

    ui->labelSerialPortLockInAmplifier->setText("Connecting");

    if (this->lockInAmplifier->autoSetLockInAmplifierType(arg1)) {
        ui->labelSerialPortLockInAmplifier->setText(this->lockInAmplifier->getLockInAmplifierType());

        if (this->check) {
            if (this->lockInAmplifier->test())
                ui->labelSerialPortLockInAmplifier->setText(ui->labelSerialPortLockInAmplifier->text() + "+");
            else
                ui->labelSerialPortLockInAmplifier->setText(ui->labelSerialPortLockInAmplifier->text() + "-");
        }

        this->lockInAmplifier->setDefaultSettings();

        if (this->lockInAmplifier->workWithTimeConstant()) {
            ui->labelTimeConstantLockInAmplifier->show();
            ui->comboBoxTimeConstantLockInAmplifier->show();
            ui->comboBoxTimeConstantLockInAmplifier->clear();
            ui->comboBoxTimeConstantLockInAmplifier->addItems(this->lockInAmplifier->getTimeConstantList());
            ui->comboBoxTimeConstantLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultTimeConstant());
            ui->labelTimeConstantLockInAmplifier->show();
        }
        ui->checkBoxAutosettingsLockInAmplifier->show();

        if (this->lockInAmplifier->workWithSensivity()) {
            ui->labelSensivityLockInAmplifier->show();
            ui->comboBoxSensivityLockInAmplifier->show();
            ui->comboBoxSensivityLockInAmplifier->clear();
            ui->comboBoxSensivityLockInAmplifier->addItems(this->lockInAmplifier->getSensivityList());
            ui->comboBoxSensivityLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultSensivity());
            ui->labelSensivityLockInAmplifier->show();
        }

        if (this->lockInAmplifier->workWithVoltageInputRange()) {
            ui->labelInputRangeLockInAmplifier->show();
            ui->comboBoxInputVoltageRangeLockInAmplifier->show();
            ui->comboBoxInputVoltageRangeLockInAmplifier->clear();
            ui->comboBoxInputVoltageRangeLockInAmplifier->addItems(this->lockInAmplifier->getVoltageInputRangeList());
            ui->comboBoxInputVoltageRangeLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultVoltageInputRange());
            ui->comboBoxInputVoltageRangeLockInAmplifier->show();
            ui->labelInputRangeLockInAmplifier->show();
        }

        if (this->generator->isActive()) {
            showAll();
        }

    } else {
        ui->labelSerialPortLockInAmplifier->setText("Not Connected!");

        ui->labelTimeConstantLockInAmplifier->hide();
        ui->comboBoxTimeConstantLockInAmplifier->hide();

        ui->checkBoxAutosettingsLockInAmplifier->hide();

        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();

        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();

        hideAll();
    }
}

void Elf::on_checkBoxAutosettingsLockInAmplifier_stateChanged(int arg1)
{
    qDebug() << "Check box autosettings was pressed";

    if (arg1 == 0) {
        ui->labelSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->show();
        if (this->lockInAmplifier->workWithVoltageInputRange()) {
            ui->labelInputRangeLockInAmplifier->show();
            ui->comboBoxInputVoltageRangeLockInAmplifier->show();
        }
    } else {
        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();
        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();

        double r = this->lockInAmplifier->getR();

        qDebug() << "R =" << r;
        ui->comboBoxSensivityLockInAmplifier->setCurrentText(this->lockInAmplifier->getAutoSensivity(r));
        ui->comboBoxInputVoltageRangeLockInAmplifier->setCurrentText(this->lockInAmplifier->getAutoVoltageInputRange(r));

        this->lockInAmplifier->setAutoSensivity(r);
        this->lockInAmplifier->setAutoVoltageInputRange(r);
    }

    return;
}

void Elf::on_comboBoxTimeConstantLockInAmplifier_currentTextChanged(const QString &arg1)
{
    qDebug() << "Current time constant for lock-in amplifier changed to" << arg1;

    this->lockInAmplifier->setTimeConstant(arg1);

    return;
}

void Elf::on_comboBoxInputVoltageRangeLockInAmplifier_currentTextChanged(const QString &arg1)
{
    qDebug() << "Current input voltage for lock-in amplifier changed to" << arg1;

    this->lockInAmplifier->setVoltageInputRange(arg1);

    return;
}

void Elf::on_comboBoxSensivityLockInAmplifier_currentTextChanged(const QString &arg1)
{
    qDebug() << "Current sensivity for lock-in amplifier changed to" << arg1;

    this->lockInAmplifier->setSensivity(arg1);

    return;
}

// Generator

void Elf::on_comboBoxSerialPortGenerator_currentTextChanged(const QString &arg1)
{
    if (this->constructor)
        return;

    if (this->generator->isActive()) {

        this->generator->disconnect();

        ui->labelSerialPortGenerator->setText("Disconnected");

        ui->labelAmplitudeGenerator->hide();
        ui->doubleSpinBoxAmplitudeGenerator->hide();

        ui->labelOffsetGenerator->hide();
        ui->doubleSpinBoxOffsetGenerator->hide();

        ui->labelFrequencyFromGenerator->hide();
        ui->doubleSpinBoxFrequencyFromGenerator->hide();

        ui->labelFrequencyToGenerator->hide();
        ui->doubleSpinBoxFrequencyToGenerator->hide();

        ui->labelFrequencyStepGenerator->hide();
        ui->doubleSpinBoxFrequencyStepGenerator->hide();

        hideAll();

        return;
    }

    ui->labelSerialPortGenerator->setText("Connecting");

    if (this->generator->autoSetGeneratorType(arg1)) {
        ui->labelSerialPortGenerator->setText(this->generator->getGeneratorType());

        if (this->check) {
            if (this->generator->test())
                ui->labelSerialPortGenerator->setText(ui->labelSerialPortGenerator->text() + "+");
            else
                ui->labelSerialPortGenerator->setText(ui->labelSerialPortGenerator->text() + "-");
        }

        this->generator->setDefaultSettings();

        if (this->generator->workWithAmplitude()) {
            ui->labelAmplitudeGenerator->show();
            ui->doubleSpinBoxAmplitudeGenerator->show();

            ui->doubleSpinBoxAmplitudeGenerator->setMinimum(this->generator->getMinAmplitude("SINE", "VR"));
            ui->doubleSpinBoxAmplitudeGenerator->setMaximum(this->generator->getMaxAmplitude("SINE", "VR"));
            ui->doubleSpinBoxAmplitudeGenerator->setSingleStep(this->generator->getStepAmplitude("SINE", "VR"));
            ui->doubleSpinBoxAmplitudeGenerator->setDecimals(this->generator->getDecimalsAmplitude("SINE", "VR"));
            ui->doubleSpinBoxAmplitudeGenerator->setValue(this->generator->getDefaultAmplitude());
        }

        if (this->generator->workWithOffset()) {
            ui->labelOffsetGenerator->show();
            ui->doubleSpinBoxOffsetGenerator->show();

            ui->doubleSpinBoxOffsetGenerator->setMinimum(this->generator->getMinOffset());
            ui->doubleSpinBoxOffsetGenerator->setMaximum(this->generator->getMaxOffset());
            ui->doubleSpinBoxOffsetGenerator->setSingleStep(this->generator->getStepOffset());
            ui->doubleSpinBoxOffsetGenerator->setDecimals(this->generator->getDecimalsOffset());
            ui->doubleSpinBoxOffsetGenerator->setValue(this->generator->getDefaultOffset());
        }

        if (this->generator->workWithFrequency()) {
            ui->labelFrequencyFromGenerator->show();
            ui->doubleSpinBoxFrequencyFromGenerator->show();

            ui->labelFrequencyToGenerator->show();
            ui->doubleSpinBoxFrequencyToGenerator->show();

            ui->labelFrequencyStepGenerator->show();
            ui->doubleSpinBoxFrequencyStepGenerator->show();

            ui->doubleSpinBoxFrequencyFromGenerator->setMinimum(this->generator->getMinFrequency("SINE"));
            ui->doubleSpinBoxFrequencyFromGenerator->setMaximum(this->generator->getMaxFrequency("SINE"));
            ui->doubleSpinBoxFrequencyFromGenerator->setSingleStep(this->generator->getStepFrequency("SINE"));
            ui->doubleSpinBoxFrequencyFromGenerator->setDecimals(this->generator->getDecimalsFrequency("SINE"));
            ui->doubleSpinBoxFrequencyFromGenerator->setValue(1E4);

            ui->doubleSpinBoxFrequencyToGenerator->setMinimum(this->generator->getMinFrequency("SINE"));
            ui->doubleSpinBoxFrequencyToGenerator->setMaximum(this->generator->getMaxFrequency("SINE"));
            ui->doubleSpinBoxFrequencyToGenerator->setSingleStep(this->generator->getStepFrequency("SINE"));
            ui->doubleSpinBoxFrequencyToGenerator->setDecimals(this->generator->getDecimalsFrequency("SINE"));
            ui->doubleSpinBoxFrequencyToGenerator->setValue(1E6);

            ui->doubleSpinBoxFrequencyStepGenerator->setMinimum(this->generator->getMinFrequency("SINE"));
            ui->doubleSpinBoxFrequencyStepGenerator->setMaximum(this->generator->getMaxFrequency("SINE"));
            ui->doubleSpinBoxFrequencyStepGenerator->setSingleStep(this->generator->getStepFrequency("SINE"));
            ui->doubleSpinBoxFrequencyStepGenerator->setDecimals(this->generator->getDecimalsFrequency("SINE"));
            ui->doubleSpinBoxFrequencyStepGenerator->setValue(1E3);
        }

        if (this->lockInAmplifier->isActive()) {
            showAll();
        }

    } else {
        ui->labelSerialPortGenerator->setText("Not Connected!");

        ui->labelAmplitudeGenerator->hide();
        ui->doubleSpinBoxAmplitudeGenerator->hide();

        ui->labelOffsetGenerator->hide();
        ui->doubleSpinBoxOffsetGenerator->hide();

        ui->labelFrequencyFromGenerator->hide();
        ui->doubleSpinBoxFrequencyFromGenerator->hide();

        ui->labelFrequencyToGenerator->hide();
        ui->doubleSpinBoxFrequencyToGenerator->hide();

        ui->labelFrequencyStepGenerator->hide();
        ui->doubleSpinBoxFrequencyStepGenerator->hide();

        hideAll();
    }
}

// Experiment

void Elf::on_comboBoxMode_currentTextChanged(const QString &arg1)
{
    qDebug() << "Current mode changed to" << arg1;

    if (arg1 == "Single") {
        ui->lcdNumber->hide();
    } else {
        ui->lcdNumber->show();
    }

    return;
}

void Elf::on_pushButtonExport_clicked()
{
    qDebug() << "Exporing data";



    return;
}

void Elf::on_pushButtonStart_clicked()
{
    qDebug() << "Starting experiment";

    double timeToFinish = this->waitBefore +
            (_round(
                (ui->doubleSpinBoxFrequencyToGenerator->value() -
                 ui->doubleSpinBoxFrequencyFromGenerator->value()) /
                 ui->doubleSpinBoxFrequencyStepGenerator->value()) + 1) *
            (this->generator->getAverageInputTime() +
             ui->spinBoxWait->value() +
             ui->spinBoxAverageOfPoints->value() *
             (this->lockInAmplifier->getAverageInputTime() +
              this->lockInAmplifier->getAverageOutputTime()));
    QTime run = QTime(0, 0, 0, 0).addMSecs(timeToFinish);
    ui->lineEditTimeToRun->setText(run.toString("hh:mm:ss.z"));

    ui->progressBarExperiment->setMaximum(_round(
                                              (ui->doubleSpinBoxFrequencyToGenerator->value() -
                                               ui->doubleSpinBoxFrequencyFromGenerator->value()) /
                                               ui->doubleSpinBoxFrequencyStepGenerator->value()) + 1);

    this->allTime = QTime::currentTime();
    this->allTime.start();

    timerStart();

    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonPause->setEnabled(true);
    ui->pushButtonStop->setEnabled(true);

    return;
}

void Elf::on_pushButtonPause_clicked()
{
    qDebug() << "Pausing experiment";

    timerPause();

    return;
}

void Elf::on_pushButtonStop_clicked()
{
    qDebug() << "Stopping experiment";

    timerStop();

    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);

    return;
}

void Elf::experimentInit()
{
    qDebug() << "Experiment initing";

    this->generator->setAmplitude(ui->doubleSpinBoxAmplitudeGenerator->value(), "VR");
    if (this->generator->workWithOffset())
        this->generator->setOffset(ui->doubleSpinBoxOffsetGenerator->value());
    this->generator->setFrequency(ui->doubleSpinBoxFrequencyFromGenerator->value());

    QTest::qWait(this->waitBefore);

    return;
}

void Elf::changeConstants()
{
    qDebug() << "Constants changed";

    this->points = ui->spinBoxAverageOfPoints->value();
    this->wait = ui->spinBoxWait->value();

    return;
}

void Elf::timerStart(const int &ms)
{
    qDebug() << "Timer starting";

    this->pause = false;
    this->stop = false;

    run.start(ms);

    return;

}

void Elf::timerPause()
{
    this->pause = !this->pause;
    this->stop = false;

    if (ui->pushButtonPause->text() == "Pause") {
        qDebug() << "Pause timer";

        run.stop();
        ui->pushButtonPause->setText("Continue");
    } else {
        qDebug() << "Continue timer";

        run.start();
        ui->pushButtonPause->setText("Pause");
    }

    return;
}

void Elf::timerStop()
{
    qDebug() << "Stop timer";

    run.stop();

    this->pause = true;
    this->stop = true;

    return;
}

void Elf::on_spinBoxAverageOfPoints_valueChanged(int arg1)
{
    qDebug() << "Average of points changed to" << arg1;

    changeConstants();

    return;
}

void Elf::on_spinBoxWait_valueChanged(int arg1)
{
    qDebug() << "Wait changed to" << arg1;

    changeConstants();

    return;
}
