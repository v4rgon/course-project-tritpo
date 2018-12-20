#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "cputools.h"
namespace cpuTools {

    std::vector<cpuStruct> getCpuTimes()
    {
        std::vector<cpuStruct> times;

        FILE* file = fopen("/proc/stat", "r");
        if (file == NULL) {
            perror("Could not open stat file");
            return times;
        }

        char buffer[1024];
        memset(buffer,1,1024);
        while(buffer[0] != '\n' && buffer != NULL) {
            buffer[0] = (char)fgetc(file);
        }

        while(buffer != NULL) {
            unsigned long long user = 0, nice = 0, system = 0, idle = 0;

            unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

            char* ret = fgets(buffer, sizeof(buffer) - 1, file);
            if (ret == NULL) {
                perror("Could not read stat file");
                fclose(file);
                return times;
            } else if (strncmp(buffer, "cpu", 3)) {
                break;
            }

            sscanf(buffer,
                   "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

            cpuStruct cpu = {idle + iowait, user + nice + system + irq + softirq + steal};
            times.push_back(cpu);
        }

        fclose(file);
        return times;
    }

    std::vector<double> calculateCpuPercentages(std::vector<cpuStruct> now, std::vector<cpuStruct> prev)
    {
        std::vector<double> times;
        if (now.size() != prev.size()) {
            return times;
        }

        for(unsigned int i=0; i<now.size(); i++) {
            cpuStruct n, p;
            n = now.at(i);
            p = prev.at(i);

            long long unsigned totald = ((n.idle + n.nonIdle) - (p.idle + p.nonIdle));
            times.push_back((totald - (n.idle - p.idle)) / (double)totald * 100.0);
        }

        return times;
    }

}
