#ifndef PROCESSTOOLS
#define PROCESSTOOLS
#include <QString>
#include <proc/readproc.h>
#include <QIcon>
#include <unordered_map>

namespace processTools {
QString getProcessName(proc_t* p);
double calculateCPUPercentage(const proc_t* before, const proc_t* after, const unsigned long long &cpuTime);
QString getProcessCmdline(pid_t pid);
QString getProcessStartDate(unsigned long long start_time);
QString getProcessStatus(proc_t* p);
unsigned long long getTotalCpuTime();
QIcon getProcessIconFromName(QString procName, std::unordered_map<QString, QIcon> &processIconMapCache);
}

#endif // PROCESSTOOLS

