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

    ui->comboBoxSerialPortLockInAmplifier->addItem("SET IT");
    ui->comboBoxSerialPortGenerator->addItem("SET IT");

    ui->comboBoxSerialPortLockInAmplifier->setCurrentText("SET IT");
    ui->comboBoxSerialPortGenerator->setCurrentText("SET IT");

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

    ui->customPlot->xAxis->setLabel(ui->comboBoxXAxisValue->currentText());
    ui->customPlot->yAxis->setLabel(ui->comboBoxYAxisValue->currentText());

    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
    ui->customPlot->axisRect()->setupFullAxesBox();

    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(on_graph_Clicked(QMouseEvent*)));

    connect(&(this->run), SIGNAL(timeout()), this, SLOT(updateGraph()));

    experimentData.clear();

    this->constructor = false;

    qDebug() << "Elf constructor finished";
}

Elf::~Elf()
{
    qDebug() << "Elf destructor started";

    stopAll();

    delete ui;

    qDebug() << "Elf destructor finished";
}

void Elf::stopAll()
{
    qDebug() << "Stopping All";

    this->stop = true;
    this->pause = true;

    this->generator->disconnect();
    this->generator->~Generator();

    this->lockInAmplifier->disconnect();
    this->lockInAmplifier->~LockInAmplifier();

    this->setAttribute(Qt::WA_DeleteOnClose, false);

    return;
}

bool Elf::inRange(const double &min, const double &max, const double &value)
{
    double _min = std::min(min, max);
    double _max = std::max(min, max);

    return (_min <= value && value <= _max);
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

//    return;

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

//    timerPause();

    ui->customPlot->xAxis->setRange(ui->lineEditRangeXmin->text().toDouble(),
                                    ui->lineEditRangeXmax->text().toDouble());
    ui->customPlot->yAxis->setRange(ui->lineEditRangeYmin->text().toDouble(),
                                    ui->lineEditRangeYmax->text().toDouble());
    ui->customPlot->replot();

//    timerPause();

    return;
}

void Elf::updateGraph()
{
    if (ui->comboBoxRange->currentText() == "Manual")
        return;

//    qDebug() << "UPDATING GRAPH";

    ui->customPlot->xAxis->setRange(ui->lineEditRangeXmin->text().toDouble() * (1-this->subDown),
                                    ui->lineEditRangeXmax->text().toDouble() * (1+this->addUp));

//    qDebug() << "3.2 SAVE";

    ui->customPlot->yAxis->setRange(ui->lineEditRangeYmin->text().toDouble() * (1-this->subDown),
                                    ui->lineEditRangeYmax->text().toDouble() * (1+this->addUp));

//    qDebug() << "3.5 SAVE";

    ui->customPlot->replot();

//    qDebug() << "3.7 SAVE";

    return;
}

void Elf::replotGraph()
{
    if (this->constructor)
        return;

    if (experimentData.isEmpty())
        return;

    qDebug() << "Plot Axises replot";

//    timerPause();

    if (experimentData.isEmpty())
        return;

    ui->customPlot->graph(0)->clearData();

    if (ui->comboBoxXAxisValue->currentText() == "Fext") {
        if (ui->comboBoxYAxisValue->currentText() == "R") {
            for (int i = 0; i < experimentData.getSize(); i++) {
                ui->customPlot->graph(0)->addData(experimentData.getFextat(i),
                                                  experimentData.getRat(i));
            }

            ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getFextMin()));
            ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getFextMax()));

            ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getRMin()));
            ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getRMax()));
        }

        if (ui->comboBoxYAxisValue->currentText() == "Theta") {
            for (int i = 0; i < experimentData.getSize(); i++) {
                ui->customPlot->graph(0)->addData(experimentData.getFextat(i),
                                                  experimentData.getThetaat(i));
            }

            ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getFextMin()));
            ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getFextMax()));

            ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getThetaMin()));
            ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getThetaMax()));
        }
    }

    if (ui->comboBoxXAxisValue->currentText() == "Time") {
        if (ui->comboBoxYAxisValue->currentText() == "R") {
            for (int i = 0; i < experimentData.getSize(); i++) {
                ui->customPlot->graph(0)->addData(experimentData.getTimeat(i),
                                                  experimentData.getRat(i));
            }

            ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getTimeMin()));
            ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getTimeMax()));

            ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getRMin()));
            ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getRMax()));
        }

        if (ui->comboBoxYAxisValue->currentText() == "Theta") {
            for (int i = 0; i < experimentData.getSize(); i++) {
                ui->customPlot->graph(0)->addData(experimentData.getTimeat(i),
                                                  experimentData.getThetaat(i));
            }

            ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getTimeMin()));
            ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getTimeMax()));

            ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getThetaMin()));
            ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getThetaMax()));
        }
    }

    on_pushButtonRangeManualreplot_clicked();

//    timerPause();

    return;
}

void Elf::pushGraph(const SimpleExperimentPoint &point)
{
//    qDebug() << "Point at graph adding";

    if (ui->comboBoxXAxisValue->currentText() == "Fext") {
        if (ui->comboBoxYAxisValue->currentText() == "R") {
            ui->customPlot->graph(0)->addData(point.Fext, point.R);

            if (ui->comboBoxRange->currentText() == "Auto") {
                ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getFextMin()));
                ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getFextMax()));

                ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getRMin()));
                ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getRMax()));
            }
        }

        if (ui->comboBoxYAxisValue->currentText() == "Theta") {
            ui->customPlot->graph(0)->addData(point.Fext, point.Theta);

            if (ui->comboBoxRange->currentText() == "Auto") {
                ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getFextMin()));
                ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getFextMax()));

                ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getThetaMin()));
                ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getThetaMax()));
            }
        }
    }

    if (ui->comboBoxXAxisValue->currentText() == "Time") {
        if (ui->comboBoxYAxisValue->currentText() == "R") {
            ui->customPlot->graph(0)->addData(point.Time, point.R);

            if (ui->comboBoxRange->currentText() == "Auto") {
                ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getTimeMin()));
                ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getTimeMax()));

                ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getRMin()));
                ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getRMax()));
            }
        }

        if (ui->comboBoxYAxisValue->currentText() == "Theta") {
            ui->customPlot->graph(0)->addData(point.Time, point.Theta);

            if (ui->comboBoxRange->currentText() == "Auto") {
                ui->lineEditRangeXmin->setText(QString::number(this->experimentData.getTimeMin()));
                ui->lineEditRangeXmax->setText(QString::number(this->experimentData.getTimeMax()));

                ui->lineEditRangeYmin->setText(QString::number(this->experimentData.getThetaMin()));
                ui->lineEditRangeYmax->setText(QString::number(this->experimentData.getThetaMax()));
            }
        }
    }

    return;
}

void Elf::on_comboBoxXAxisValue_currentTextChanged(const QString &arg1)
{
    ui->customPlot->xAxis->setLabel(arg1);

    ui->customPlot->replot();

    replotGraph();

    return;
}

void Elf::on_comboBoxYAxisValue_currentTextChanged(const QString &arg1)
{
    ui->customPlot->yAxis->setLabel(arg1);

    ui->customPlot->replot();

    replotGraph();

    return;
}

// Lock-in Amplifier

void Elf::on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1)
{
    if (this->constructor)
        return;

    if (arg1 == "SET IT")
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

//        return;
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

//        if (this->generator->isActive()) {
            showAll();
//        }

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

//        hideAll();

//        return;
    }

    this->generatorActive = false;

    ui->labelSerialPortGenerator->setText("Connecting");

    if (arg1 != "SET IT" && this->generator->autoSetGeneratorType(arg1)) {
        ui->labelSerialPortGenerator->setText(this->generator->getGeneratorType());

        if (this->check) {
            if (this->generator->test())
                ui->labelSerialPortGenerator->setText(ui->labelSerialPortGenerator->text() + "+");
            else
                ui->labelSerialPortGenerator->setText(ui->labelSerialPortGenerator->text() + "-");
        }

//        this->generator->setDefaultSettings();

        this->generatorActive = true;

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

            ui->doubleSpinBoxFrequencyStepGenerator->setMinimum(-this->generator->getMaxFrequency("SINE"));
//            ui->doubleSpinBoxFrequencyStepGenerator->setMinimum(this->generator->getMinFrequency("SINE"));
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

//        hideAll();
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

    changeConstants();

    return;
}

void Elf::changeConstants()
{
    qDebug() << "Constants changed";

    timerPause();

    this->points = ui->spinBoxAverageOfPoints->value();
    this->wait = ui->spinBoxWait->value();

    if (this->generatorActive) {
        this->from = ui->doubleSpinBoxFrequencyFromGenerator->value();
        this->to   = ui->doubleSpinBoxFrequencyToGenerator->value();
        this->step = ui->doubleSpinBoxFrequencyStepGenerator->value();
    } else {
        this->from = this->to = this->step = 0;
    }

    this->continuous = (ui->comboBoxMode->currentText() == "Continuous");

    timerPause();

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
    if (!ui->pushButtonPause->isEnabled())
        return;

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

QString Elf::getFileName(const QString &header)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString("yyyy_MM_dd_HH_mm_ss");

    qDebug() << "File name from header " <<  header + "_" + dateTimeString + ".dat";

    return header + "_" + dateTimeString + ".dat";
}

void Elf::on_pushButtonExport_clicked()
{
    qDebug() << "Exporing data";

    QString fileName = getFileName(this->currentFolder.absolutePath() + "\\Data\\" + ui->lineEditFileHeader->text());

    qDebug() << "Exporting to file:" << fileName;

    fclose(stdout);
    freopen(fileName.toStdString().c_str(), "w", stdout);
    printf("Fext\tFextSD\tFgen\tR\tRSD\tTheta\tThetaSD\tTime\n");
    for (int i = 0; i < experimentData.getSize(); i++) {
        printf("%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\n",
               experimentData.getFextat(i),
               experimentData.getFextSDat(i),
               experimentData.getFgenat(i),
               experimentData.getRat(i),
               experimentData.getRSDat(i),
               experimentData.getThetaat(i),
               experimentData.getThetaSDat(i),
               experimentData.getTimeat(i));
    }
    fclose(stdout);

    qDebug() << "Exporing settings";

    fileName = getFileName(this->currentFolder.absolutePath() + "\\Data\\" + ui->lineEditFileHeader->text() + "_Experiment_Settings");

    qDebug() << "Exporting to file:" << fileName;

    fclose(stdout);
    freopen(fileName.toStdString().c_str(), "w", stdout);

    std::cout << "Current experiment settings:" << std::endl;
    std::cout << std::endl;

    if (!this->generatorActive) {
        std::cout << "Generator - not active" << std::endl;
    } else {
        std::cout << "Generator model: " << ui->labelSerialPortGenerator->text().toStdString() << std::endl;
        std::cout << "   Offset: " << ui->doubleSpinBoxOffsetGenerator->value() << " V" << std::endl;
        std::cout << "   Amplitude: " << ui->doubleSpinBoxAmplitudeGenerator->value() << " Vrms" << std::endl;
        std::cout << "   Frequency from: " << ui->doubleSpinBoxFrequencyFromGenerator->value() << " Hz" << std::endl;
        std::cout << "   Frequency to:   " << ui->doubleSpinBoxFrequencyToGenerator->value() << " Hz" << std::endl;
        std::cout << "   Frequency step: " << ui->doubleSpinBoxFrequencyStepGenerator->value() << " Hz" << std::endl;
        if (ui->checkBoxPollGenerator->isChecked())
            std::cout << "GENERATOR POLLING IS ON" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Lock-in amplifier model: " << ui->labelSerialPortLockInAmplifier->text().toStdString() << std::endl;
    std::cout << "   Time constant: " << ui->comboBoxTimeConstantLockInAmplifier->currentText().toStdString() << std::endl;
    if (this->lockInAmplifier->workWithVoltageInputRange())
        std::cout << "   Input range:   " << ui->comboBoxInputVoltageRangeLockInAmplifier->currentText().toStdString() <<  std::endl;
    std::cout << "   Sensivity:     " << ui->comboBoxSensivityLockInAmplifier->currentText().toStdString() <<  std::endl;
    if (ui->checkBoxAutosettingsLockInAmplifier->isChecked())
        std::cout << "   LOCK-IN AMPLIFIER AUTOSETTINGS CHECKED" << std::endl;

    std::cout << std::endl;
    std::cout << "Experiment:" << std::endl;
    std::cout << "   Mode: " << ui->comboBoxMode->currentText().toStdString() << std::endl;
    if (ui->comboBoxMode->currentText() == "Continuous")
        std::cout << "       Last round was " << ui->lcdNumber->value() << std::endl;
    std::cout << "   Average of " << ui->spinBoxAverageOfPoints->value() << " points" << std::endl;
    std::cout << "   For new point wait for " << ui->spinBoxWait->value() << " ms" << std::endl;

    fclose(stdout);

    return;
}

void Elf::on_pushButtonStart_clicked()
{
    qDebug() << "Starting experiment";

    this->setAttribute(Qt::WA_DeleteOnClose, true);

    ui->customPlot->graph(0)->clearData();

    double timeToFinish = this->waitBefore +
            (_round(
                (ui->doubleSpinBoxFrequencyToGenerator->value() -
                 ui->doubleSpinBoxFrequencyFromGenerator->value()) /
                 ui->doubleSpinBoxFrequencyStepGenerator->value()) +
             1 + this->invalidPoints) *
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
    if (!this->generatorActive) {
        ui->progressBarExperiment->setMaximum(1);
        ui->progressBarExperiment->setValue(0);
    }

    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonPause->setEnabled(true);
    ui->pushButtonStop->setEnabled(true);
    ui->pushButtonExport->setEnabled(true);

    ui->lcdNumber->display(0);

    experimentData.clear();

    this->allTime = QTime::currentTime();
    this->allTime.start();

    this->startTime = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    timerStart(this->updateTime);

    experiment_Run();

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

    this->setAttribute(Qt::WA_DeleteOnClose, false);

    timerStop();

    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);

    return;
}

void Elf::experiment_StartingPoint()
{
    qDebug() << "Experiment starting points settings";

    if (!this->generatorActive)
        return;

    this->generator->setOffset(ui->doubleSpinBoxOffsetGenerator->value());
    this->generator->setAmplitude(ui->doubleSpinBoxAmplitudeGenerator->value(), "VR");
    this->generator->setFrequency(ui->doubleSpinBoxFrequencyFromGenerator->value());

    return;
}

void Elf::experiment_Run()
{
    qDebug() << "Running an experiment";

    changeConstants();

    double R = 0;
    double RSD = 0;
    double Theta = 0;
    double ThetaSD = 0;
    double F = 0;
    double FSD = 0;

    SimpleExperimentPoint new_point;

    std::vector < double > RSDvector(this->points);
    std::vector < double > ThetaSDvector(this->points);
    std::vector < double > FSDvector(this->points);

    qDebug() << "Exporing settings";

    QString fileName = getFileName(this->currentFolder.absolutePath() + "\\Data\\Reserve\\" + ui->lineEditFileHeader->text() + "_Experiment_Settings");

    qDebug() << "Exporting to file:" << fileName;

    fclose(stdout);
    freopen(fileName.toStdString().c_str(), "w", stdout);

    std::cout << "Current experiment settings:" << std::endl;
    std::cout << std::endl;

    if (!this->generatorActive) {
        std::cout << "Generator - not active" << std::endl;
    } else {
        std::cout << "Generator model: " << ui->labelSerialPortGenerator->text().toStdString() << std::endl;
        std::cout << "   Offset: " << ui->doubleSpinBoxOffsetGenerator->value() << " V" << std::endl;
        std::cout << "   Amplitude: " << ui->doubleSpinBoxAmplitudeGenerator->value() << " Vrms" << std::endl;
        std::cout << "   Frequency from: " << ui->doubleSpinBoxFrequencyFromGenerator->value() << " Hz" << std::endl;
        std::cout << "   Frequency to:   " << ui->doubleSpinBoxFrequencyToGenerator->value() << " Hz" << std::endl;
        std::cout << "   Frequency step: " << ui->doubleSpinBoxFrequencyStepGenerator->value() << " Hz" << std::endl;
        if (ui->checkBoxPollGenerator->isChecked())
            std::cout << "GENERATOR POLLING IS ON" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Lock-in amplifier model: " << ui->labelSerialPortLockInAmplifier->text().toStdString() << std::endl;
    std::cout << "   Time constant: " << ui->comboBoxTimeConstantLockInAmplifier->currentText().toStdString() << std::endl;
    if (this->lockInAmplifier->workWithVoltageInputRange())
        std::cout << "   Input range:   " << ui->comboBoxInputVoltageRangeLockInAmplifier->currentText().toStdString() <<  std::endl;
    std::cout << "   Sensivity:     " << ui->comboBoxSensivityLockInAmplifier->currentText().toStdString() <<  std::endl;
    if (ui->checkBoxAutosettingsLockInAmplifier->isChecked())
        std::cout << "   LOCK-IN AMPLIFIER AUTOSETTINGS CHECKED" << std::endl;

    std::cout << std::endl;
    std::cout << "Experiment:" << std::endl;
    std::cout << "   Mode: " << ui->comboBoxMode->currentText().toStdString() << std::endl;
    if (ui->comboBoxMode->currentText() == "Continuous")
        std::cout << "       Last round was " << ui->lcdNumber->value() << std::endl;
    std::cout << "   Average of " << ui->spinBoxAverageOfPoints->value() << " points" << std::endl;
    std::cout << "   For new point wait for " << ui->spinBoxWait->value() << " ms" << std::endl;

    fclose(stdout);

    QString reserveFileName = getFileName(this->reserveFileNameHeader);
    freopen(reserveFileName.toStdString().c_str(), "w", stdout);
    printf("Fext\tFextSD\tFgen\tR\tRSD\tTheta\tThetaSD\tTime\n");

    setbuf(stdout, NULL); // DISABLE BUFERING

    double generatorFrequency;

    ui->lineEditRangeXmin->setText(QString::number(0));
    ui->lineEditRangeXmax->setText(QString::number(0));
    ui->lineEditRangeYmin->setText(QString::number(0));
    ui->lineEditRangeYmax->setText(QString::number(0));

    do {
        ui->lcdNumber->display((ui->lcdNumber->intValue() + 1) % 100);
        ui->progressBarExperiment->setValue(0);

        experiment_StartingPoint();
        QTest::qWait(this->waitBefore);

        for (int i = 0; i < this->invalidPoints; i++) {
            QTest::qWait(this->wait);
            this->lockInAmplifier->getRThetaFext(R, Theta, F);
        }

        generatorFrequency = this->from;
        while (inRange(this->from, this->to, generatorFrequency)) {
            R = F = Theta = 0;
            RSD = FSD = ThetaSD = 0;

            new_point.Fext = 0;
            new_point.FextSD = 0;
            new_point.Fgen = generatorFrequency;
            new_point.R = 0;
            new_point.RSD = 0;
            new_point.Theta = 0;
            new_point.ThetaSD = 0;
            new_point.Time = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - this->startTime;

            QTime run = QTime(0, 0, 0, 0).addMSecs(new_point.Time*1000);
            ui->lineEditTimePassed->setText(run.toString("hh:mm:ss.z"));

//            qDebug() << "1 SAVE";

            for (int point = 0; point < this->points; point++) {
//                this->lockInAmplifier->getRThetaFext(R, Theta, F);
                R = this->lockInAmplifier->getR();

//                R = std::sin(new_point.Time);
//                Theta = 1 - R;
//                F = 1 - new_point.Time;

                RSDvector[point] = R;
                ThetaSDvector[point] = Theta;
                FSDvector[point] = F;

                new_point.R += R;
                new_point.Theta += Theta;
                new_point.Fext += F;
            }

            new_point.R /= this->points;
            new_point.Theta /= this->points;
            new_point.Fext /= this->points;

//            qDebug() << "2 SAVE";

            for (int point = 0; point < this->points; point++) {
                RSD += _sqr(RSDvector[point] - new_point.R);
                ThetaSD += _sqr(ThetaSDvector[point] - new_point.Theta);
                FSD += _sqr(FSDvector[point] - new_point.Fext);
            }

            RSD = _sqrt(RSD/this->points);
            ThetaSD = _sqrt(ThetaSD/this->points);
            FSD = _sqrt(FSD/this->points);

            new_point.FextSD = FSD;
            new_point.RSD = RSD;
            new_point.ThetaSD = ThetaSD;

//            qDebug() << "3 SAVE";

            ui->lineEditCurrentReadingsFrequencyGenerator->setText(QString::number(generatorFrequency));
            ui->lineEditCurrentReadingsRLockInAmplifier->setText(QString::number(new_point.R, 'g', 6) + " +- " +
                                                                 QString::number(100*new_point.RSD/new_point.R, 'g', 2) + "%");
            ui->lineEditCurrentReadingsThetaLockInAmplifier->setText(QString::number(new_point.Theta, 'g', 6) + " +- " +
                                                                     QString::number(100*new_point.ThetaSD/new_point.Theta, 'g', 2) + "%");
            ui->lineEditCurrentReadingsExternalFrequencyLockInAmplifier->setText(QString::number(new_point.Fext, 'g', 6) + " +- " +
                                                                                 QString::number(100*new_point.FextSD/new_point.Fext, 'g', 2) + "%");

            if (this->generatorActive)
                ui->progressBarExperiment->setValue(ui->progressBarExperiment->value() + 1);

            experimentData.push_back(new_point);
            pushGraph(new_point);

            printf("%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\t%0.20e\n",
                   new_point.Fext,
                   new_point.FextSD,
                   new_point.Fgen,
                   new_point.R,
                   new_point.RSD,
                   new_point.Theta,
                   new_point.ThetaSD,
                   new_point.Time);

            updateGraph();

//            qDebug() << "4 SAVE";

            if (this->stop) {
                return;
            }

            while (this->pause) {
                QTest::qWait(10);
                if (this->stop) {
                    return;
                }
            }

            generatorFrequency += this->step;

            if (this->generatorActive) {
                if (ui->checkBoxPollGenerator->isChecked()) {
                    for (int send = 0; send < this->generator_send; send++) {
                        this->generator->setFrequency(generatorFrequency);
                        QTest::qWait(50);
                        if (_abs(this->generator->getFrequency() - generatorFrequency < this->step))
                            break;
                    }
                } else {
                    this->generator->setFrequency(generatorFrequency);
                }
            }

            QTest::qWait(this->wait);
        }

        ui->progressBarExperiment->setValue(ui->progressBarExperiment->maximum());
    } while (this->continuous);

    fclose(stdout);
    on_pushButtonStop_clicked();

    return;
}
