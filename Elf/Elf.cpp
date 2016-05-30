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

    this->constructor = false;

    qDebug() << "Elf constructor finished";
}

Elf::~Elf()
{
    qDebug() << "Elf destructor started";

    delete ui;

    qDebug() << "Elf destructor finished";
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
    } else {
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

void Elf::on_comboBoxSerialPortLockInAmplifier_currentTextChanged(const QString &arg1)
{
    if (this->constructor)
        return;

    if (this->lockInAmplifier->autoSetLockInAmplifierType(arg1, QSerialPort::Baud19200)) {
        ui->labelSerialPortLockInAmplifier->setText(this->lockInAmplifier->getLockInAmplifierType());

        ui->labelTimeConstantLockInAmplifier->show();
        ui->comboBoxTimeConstantLockInAmplifier->show();
        ui->comboBoxTimeConstantLockInAmplifier->clear();
        ui->comboBoxTimeConstantLockInAmplifier->addItems(this->lockInAmplifier->getTimeConstantList());

        ui->checkBoxAutosettingsLockInAmplifier->show();

        ui->labelSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->clear();
        ui->comboBoxSensivityLockInAmplifier->addItems(this->lockInAmplifier->getSensivityList());

        ui->labelInputRangeLockInAmplifier->show();
        ui->comboBoxInputVoltageRangeLockInAmplifier->show();
        ui->comboBoxInputVoltageRangeLockInAmplifier->clear();
        ui->comboBoxInputVoltageRangeLockInAmplifier->addItems(this->lockInAmplifier->getVoltageInputRangeList());

    } else {
        ui->labelSerialPortLockInAmplifier->setText("Not Connected!");

        ui->labelTimeConstantLockInAmplifier->hide();
        ui->comboBoxTimeConstantLockInAmplifier->hide();

        ui->checkBoxAutosettingsLockInAmplifier->hide();

        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();

        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();
    }
}

void Elf::on_checkBoxAutosettingsLockInAmplifier_stateChanged(int arg1)
{
    qDebug() << "Check box autosettings was pressed";

    if (arg1 == 0) {
        ui->labelSensivityLockInAmplifier->show();
        ui->comboBoxSensivityLockInAmplifier->show();
        ui->labelInputRangeLockInAmplifier->show();
        ui->comboBoxInputVoltageRangeLockInAmplifier->show();
    } else {
        ui->labelSensivityLockInAmplifier->hide();
        ui->comboBoxSensivityLockInAmplifier->hide();
        ui->labelInputRangeLockInAmplifier->hide();
        ui->comboBoxInputVoltageRangeLockInAmplifier->hide();
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
