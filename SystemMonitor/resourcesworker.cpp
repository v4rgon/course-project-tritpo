#include "resourcesworker.h"
#include <proc/sysinfo.h>
#include <iostream>
#include <string>
#include "cputools.h"
#include <ctime>
using namespace cpuTools;

resourcesWorker::resourcesWorker(QObject *parent, QSettings *settings)
    : QObject(parent), workerThread()
{

    this->settings = settings;
}

resourcesWorker::~resourcesWorker()
{

}

