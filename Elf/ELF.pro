#-------------------------------------------------
#
# Project created by QtCreator 2016-05-29T11:27:32
#
#-------------------------------------------------

QT       += core gui serialport testlib
CONFIG   += c++11

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT.testlib.CONFIG -= console

#INCLUDEPATH += C:/Qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

TARGET = ELF
TEMPLATE = app


SOURCES += main.cpp\
        Elf.cpp \
    ../ANGEL/Equipment/CommonFunctionsForSRS/SRS.cpp \
    ../ANGEL/Equipment/Generator/DS335/DS335.cpp \
    ../ANGEL/Equipment/Generator/DS345/DS345.cpp \
    ../ANGEL/Equipment/Generator/Generator/Generator.cpp \
    ../ANGEL/Equipment/LockInAmplifier/LockInAmplifier/LockInAmplifier.cpp \
    ../ANGEL/Equipment/LockInAmplifier/SR830/SR830.cpp \
    ../ANGEL/Equipment/LockInAmplifier/SR844/SR844.cpp \
    ../ANGEL/Equipment/LockInAmplifier/SR865/SR865.cpp \
    ../ANGEL/Functions/AllFunctions.cpp \
    qcustomplot.cpp

HEADERS  += Elf.h \
    ../ANGEL/DataStructures/DataSharing/TDataSharing.h \
    ../ANGEL/DataStructures/ExperimentData/ExperimentData.h \
    ../ANGEL/DataStructures/ExperimentData/SimpleExperiment.h \
    ../ANGEL/DataStructures/ExperimentData/Kinetics.h \
    ../ANGEL/DataStructures/ExperimentData/DataVector.h \
    ../ANGEL/DataStructures/LockFreeQueue/TLockFreeQueue.h \
    ../ANGEL/DataStructures/AllDataStructures.h \
    ../ANGEL/Equipment/CommonFunctionsForSRS/SRS.h \
    ../ANGEL/Equipment/Generator/DS335/DS335.h \
    ../ANGEL/Equipment/Generator/DS345/DS345.h \
    ../ANGEL/Equipment/Generator/Generator/Generator.h \
    ../ANGEL/Equipment/Generator/AllGenerators.h \
    ../ANGEL/Equipment/LockInAmplifier/LockInAmplifier/LockInAmplifier.h \
    ../ANGEL/Equipment/LockInAmplifier/SR830/SR830.h \
    ../ANGEL/Equipment/LockInAmplifier/SR844/SR844.h \
    ../ANGEL/Equipment/LockInAmplifier/SR865/SR865.h \
    ../ANGEL/Equipment/LockInAmplifier/AllLockInAmplifiers.h \
    ../ANGEL/Equipment/AllEquipment.h \
    ../ANGEL/Functions/AllFunctions.h \
    ../ANGEL/Angel.h \
    qcustomplot.h

FORMS    += Elf.ui
