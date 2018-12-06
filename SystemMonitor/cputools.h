#ifndef CPUTOOLS_H
#define CPUTOOLS_H

namespace cpuTools {
typedef struct cpuStruct {
    long long unsigned idle, nonIdle;
} cpuStruct;
std::vector<cpuStruct> getCpuTimes();
std::vector<double> calculateCpuPercentages(std::vector<cpuStruct> now, std::vector<cpuStruct> prev);
}

#endif // CPUTOOLS_H
