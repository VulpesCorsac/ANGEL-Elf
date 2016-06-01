#include "Elf.h"
#include "ui_Elf.h"

Elf::Elf(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Elf)
{
    qDebug() << "Elf constructor started";

    this->constructor = true;

    ui->setupUi(this);

    this->lockInAmplifier = new LockInAmplifier();
    this->generator = new Generator();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBoxSerialPortLockInAmplifier->addItem(info.portName());
        ui->comboBoxSerialPortGenerator->addItem(info.portName());
    }


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

    ui->groupBoxExpriment->hide();
    ui->groupBoxGraph->hide();
    ui->groupBoxTiming->hide();
    ui->groupBoxCurrentReadings->hide();
//*/

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

    ui->customPlot->replot();

//    ui->customPlot->xAxis->setLabel(ui->comboBoxXAxis->currentText());
//    ui->customPlot->yAxis->setLabel(ui->comboBoxYAxis->currentText());

    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
    ui->customPlot->axisRect()->setupFullAxesBox();

    ui->customPlot->hide();

    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(on_graph_Clicked(QMouseEvent*)));

    J4F();

    this->constructor = false;

    qDebug() << "Elf constructor finished";
}

Elf::~Elf()
{
    qDebug() << "Elf destructor started";

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

void Elf::on_graph_Clicked(QMouseEvent *event)
{
    double x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    ui->lineEditPointX->setText(QString::number(x));
    ui->lineEditPointY->setText(QString::number(y));

    return;
}

void Elf::showAll() {

    ui->groupBoxExpriment->show();
    ui->groupBoxGraph->show();
    ui->groupBoxTiming->show();
    ui->groupBoxCurrentReadings->show();

    ui->customPlot->show();

    return;
}

void Elf::on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1)
{
    if (this->constructor)
        return;

    ui->labelSerialPortLockInAmplifier->setText("Connecting");

    if (this->lockInAmplifier->autoSetLockInAmplifierType(arg1)) {
        ui->labelSerialPortLockInAmplifier->setText(this->lockInAmplifier->getLockInAmplifierType());

        this->lockInAmplifier->setDefaultSettings();

        ui->labelTimeConstantLockInAmplifier->show();
        ui->comboBoxTimeConstantLockInAmplifier->show();
        ui->comboBoxTimeConstantLockInAmplifier->clear();
        ui->comboBoxTimeConstantLockInAmplifier->addItems(this->lockInAmplifier->getTimeConstantList());
        ui->comboBoxTimeConstantLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultTimeConstant());

        ui->checkBoxAutosettingsLockInAmplifier->show();

        ui->labelSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->clear();
        ui->comboBoxSensivityLockInAmplifier->addItems(this->lockInAmplifier->getSensivityList());
        ui->comboBoxSensivityLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultSensivity());

        ui->labelInputRangeLockInAmplifier->show();
        ui->comboBoxInputVoltageRangeLockInAmplifier->show();
        ui->comboBoxInputVoltageRangeLockInAmplifier->clear();
        ui->comboBoxInputVoltageRangeLockInAmplifier->addItems(this->lockInAmplifier->getVoltageInputRangeList());
        ui->comboBoxInputVoltageRangeLockInAmplifier->setCurrentText(this->lockInAmplifier->getDefaultVoltageInputRange());

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

        ui->groupBoxExpriment->hide();
        ui->groupBoxGraph->hide();
        ui->groupBoxTiming->hide();
        ui->groupBoxCurrentReadings->hide();

        ui->customPlot->hide();
    }
}

void Elf::on_checkBoxAutosettingsLockInAmplifier_stateChanged(int arg1)
{
    qDebug() << "Check box autosettings was pressed";

    if (arg1 == 0) {
        ui->labelSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->show();
        ui->labelInputRangeLockInAmplifier->show();
        if (this->lockInAmplifier->workWithVoltageInputRange())
            ui->comboBoxInputVoltageRangeLockInAmplifier->show();
    } else {
        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();
        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();

        double r = this->lockInAmplifier->getR();

        qDebug() << "R ==" << r;
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
