#include "resourcesworker.h"
#include <proc/sysinfo.h>
#include <iostream>
#include <string>
#include "cputools.h"
#include "memoryconverter.h"
#include <ctime>
#include "colourhelper.h"
using namespace cpuTools;
using namespace colourHelper;

resourcesWorker::resourcesWorker(QObject *parent, QSettings *settings)
    : QObject(parent), workerThread()
{
    memoryBar = parent->findChild<QProgressBar*>("memoryBar");
    memoryLabel = parent->findChild<QLabel*>("memoryLabel");
    swapBar = parent->findChild<QProgressBar*>("swapBar");
    swapLabel = parent->findChild<QLabel*>("swapLabel");


    connect(this,SIGNAL(updateMemoryBar(int)),memoryBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateMemoryText(QString)),memoryLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateSwapBar(int)),swapBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateSwapText(QString)),swapLabel,SLOT(setText(QString)));


    this->settings = settings;
}

resourcesWorker::~resourcesWorker()
{

}

void resourcesWorker::updateCpu()
{
    std::vector<cpuStruct> cpuTimes = getCpuTimes();
    if (prevCpuTimes.size() != 0) {
        std::vector<double> cpuPercentages = calculateCpuPercentages(cpuTimes, prevCpuTimes);

        if (cpuPlotData.size() == 60) {
            cpuPlotData.pop_front();
        }
        cpuPlotData.push_back(cpuPercentages);
    }
    prevCpuTimes = cpuTimes;

    QVector<QVector<double>> plottingData = QVector<QVector<double>>();


    if (cpuPlotData.size() == 0) {
        return;
    }

    for(unsigned int i=0; i<cpuPlotData.at(0).size(); i++) {
        QVector<double> headingVector;
        headingVector.push_back(cpuPlotData.at(0).at(i));
        plottingData.push_back(headingVector);
    }


    for(unsigned int i=1; i<cpuPlotData.size(); i++) {
        for(unsigned int j=0; j<cpuPlotData.at(i).size(); j++) {
            plottingData[j].push_back(cpuPlotData.at(i).at(j));
        }
    }


    for(int i=0; i<plottingData.size(); i++) {
        for(unsigned int j=60 - plottingData.at(i).size(); j>0; j--) {
            plottingData[i].push_front((double)0);
        }
    }

    emit(updateCpuPlotSIG(plottingData));
}

void resourcesWorker::createColourDialogue()
{
    QColor defaultColour = colourHelper::createColourFromSettings(settings, sender()->objectName(), defaultColours[sender()->objectName()].array);
    QColor newColour = QColorDialog::getColor(defaultColour, (QWidget*) this->parent());
    if (newColour.isValid()) {
        colourHelper::saveColourToSettings(settings, sender()->objectName(), newColour);
    }
}

void resourcesWorker::updateMemory()
{
    memoryConverter mainUsed;

    if (settings->value("cachedMemoryIsUsed", false).toBool()) {
        mainUsed = memoryConverter(kb_main_used + kb_main_buffers + kb_main_cached - kb_main_shared,memoryUnit::kb,standard);
    } else {
        mainUsed = memoryConverter(kb_main_used,memoryUnit::kb,standard);
    }

    memoryConverter mainTotal = memoryConverter(kb_main_total,memoryUnit::kb,standard);

    memoryConverter mainUsedCopy = mainUsed;
    mainUsedCopy.convertTo(mainTotal.getUnit());
    double memory = (mainUsedCopy.getValue() / mainTotal.getValue()) * 100;
    emit(updateMemoryBar(memory));
    std::string memPercent = memoryConverter::dbl2str(memoryConverter::truncateDouble(memoryConverter::roundDouble(memory, 1),1));

    std::string memoryText = mainUsed.to_string() + " (" + memPercent + "%) of " + mainTotal.to_string();
    emit(updateMemoryText(QString::fromStdString(memoryText)));
}

void resourcesWorker::updateSwap()
{
    if (kb_swap_total > 0.0) {

        double swap = ((double)kb_swap_used / kb_swap_total) * 100;
        emit(updateSwapBar(swap));

        memoryConverter swapUsed = memoryConverter(kb_swap_used,memoryUnit::kb,standard);
        memoryConverter swapTotal = memoryConverter(kb_swap_total,memoryUnit::kb,standard);


        std::string swapPercent = memoryConverter::dbl2str(memoryConverter::truncateDouble(memoryConverter::roundDouble(swap, 1),1));

        std::string swapText = swapUsed.to_string() + " (" + swapPercent + "%) of " + swapTotal.to_string();
        emit(updateSwapText(QString::fromStdString(swapText)));
    } else {

        emit(updateSwapBar(0));
        emit(updateSwapText("Not Available"));
    }
}


void resourcesWorker::loop()
{
    clock_t begin = clock();

    if (settings->value("resources update interval",1.0).toDouble() < 0.25) {
        settings->setValue("resources update interval",0.25);
    }

    meminfo();

    standard = memoryConverter::stringToStandard(settings->value("unit prefix standards", JEDEC).toString().toStdString());

    updateCpu();
    updateMemory();
    updateSwap();

    clock_t end = clock();
    double elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;

    double waitTime = settings->value("resources update interval",1.0).toDouble() - elapsedSecs;

    if (waitTime >= 0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(waitTime* 1000));
    }
}
