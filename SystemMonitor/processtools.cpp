#include "processtools.h"
#include <string>
#include <unistd.h>
#include <fstream>
#include <QFileInfo>
#include <proc/sysinfo.h>
#include <time.h>
#include <qdiriterator.h>
#include <QIcon>
#include <QMap>
#include <unordered_set>
#include <unordered_map>
#include "hashqstring.h"

namespace processTools {

    char* exe_of(const pid_t pid, size_t *const sizeptr, size_t *const lenptr)
    {
        char   *exe_path = NULL;
        size_t  exe_size = 1024;
        ssize_t exe_used;
        char    path_buf[64];
        unsigned int path_len;

        path_len = snprintf(path_buf, sizeof path_buf, "/proc/%ld/exe", (long)pid);
        if (path_len < 1 || path_len >= sizeof path_buf) {
            errno = ENOMEM;
            return NULL;
        }

        while (1) {

            exe_path = (char*)malloc(exe_size);
            if (!exe_path) {
                errno = ENOMEM;
                return NULL;
            }

            exe_used = readlink(path_buf, exe_path, exe_size - 1);
            if (exe_used == (ssize_t)-1) {
                free(exe_path);
                return NULL;
            }

            if (exe_used < (ssize_t)1) {
                errno = ENOENT;
                free(exe_path);
                return NULL;
            }

            if (exe_used < (ssize_t)(exe_size - 1))
                break;

            free(exe_path);
            exe_size += 1024;
        }
        {
            char *temp;

            temp = (char*)realloc(exe_path, exe_used + 1);
            if (temp) {
                exe_path = temp;
                exe_size = exe_used + 1;
            }
        }

        if (sizeptr)
            *sizeptr = exe_size;

        if (lenptr)
            *lenptr = exe_used;

        exe_path[exe_used] = '\0';
        return exe_path;
    }

    const std::vector<std::string> explode(const std::string& s, const char& c)
    {
        std::string buff{""};
        std::vector<std::string> v;

        for(auto n:s)
        {
            if(n != c) buff+=n; else
            if(n == c && buff != "") { v.push_back(buff); buff = ""; }
        }
        if(buff != "") v.push_back(buff);

        return v;
    }

    QString getProcessNameFromCmdLine(const pid_t pid)
    {
        std::string cmdline = getProcessCmdline(pid).toStdString();

        if (cmdline.size()<1) {
            return "";
        }
        std::replace(cmdline.begin(),cmdline.end(),'\\','/');

        auto args = explode(cmdline,' ');
        QString name = QFileInfo(QString::fromStdString(args[0])).fileName();
        static std::unordered_set<QString> nameMap({"python", "python2", "python3", "ruby", "php", "perl"});
        auto pos = nameMap.find(name);
        if (pos != nameMap.end()) {
            return QFileInfo(QString::fromStdString(args[1])).fileName();
        } else {
            return name;
        }
    }

    QString getProcessCmdline(pid_t pid)
    {
        std::string temp;
        try {
            std::fstream fs;
            fs.open("/proc/"+std::to_string((long)pid)+"/cmdline", std::fstream::in);
            std::getline(fs,temp);
            fs.close();
        } catch(std::ifstream::failure e) {
            return "FAILED TO READ PROC";
        }

        // change \0 to ' '
        std::replace(temp.begin(),temp.end(),'\0',' ');

        if (temp.size()<1) {
            return "";
        }
        return QString::fromStdString(temp);
    }

    QString getProcessName(proc_t* p)
    {
        QString processName = "ERROR";
        char* temp = NULL;//exe_of(p->tid,NULL,NULL);
        if (temp!=NULL) {
            processName = QFileInfo(temp).fileName();
            free(temp);
        } else {
            processName = getProcessNameFromCmdLine(p->tid);
            if (processName=="") {
                processName = p->cmd;
            }
        }
        return processName;
    }

    unsigned long long getTotalCpuTime()
    {
        FILE* file = fopen("/proc/stat", "r");
        if (file == NULL) {
            perror("Could not open stat file");
            return 0;
        }

        char buffer[1024];
        unsigned long long user = 0, nice = 0, system = 0, idle = 0;
        unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

        char* ret = fgets(buffer, sizeof(buffer) - 1, file);
        if (ret == NULL) {
            perror("Could not read stat file");
            fclose(file);
            return 0;
        }
        fclose(file);

        sscanf(buffer,
               "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

   double calculateCPUPercentage(const proc_t* before, const proc_t* after, const unsigned long long &cpuTime)
    {
        double cpuTimeA = getTotalCpuTime() - cpuTime;
        unsigned long long processcpuTime = ((after->utime + after->stime)
                - (before->utime + before->stime));
        return (processcpuTime / cpuTimeA) * 100.0 * sysconf(_SC_NPROCESSORS_CONF);
    }


    QString getProcessStartDate(unsigned long long start_time)
    {
        __time_t time = getbtime() + start_time/Hertz;
        struct tm *tm = localtime(&time);
        char date[255];
        strftime(date, sizeof(date), "%Ex %ER", tm);
        return QString(date);
    }

    QString getProcessStatus(proc_t* p)
    {
        switch(p->state) {
            case 'S':
                return "Sleeping";
            break;

            case 'R':
                return "Running";
            break;

            case 'Z':
                return "Zombie";
            break;

            case 'D':
                return "Uninterruptible Sleep";
            break;

            case 'T':
                return "Stopped";
            break;

            default:
                return "Unknown state: '" + QString(p->state) + "'";
        }
    }


    QIcon getProcessIconFromName(QString procName, std::unordered_map<QString, QIcon> &processIconMapCache)
    {
        auto pos = processIconMapCache.find(procName);
        if (pos != processIconMapCache.end()) {
            return pos->second;
        }
        static std::map<QString, QString> procNameCorrections({
            {"sh","terminal"}, {"bash","terminal"}, {"dconf-service", "dconf"}, {"gconfd-2", "dconf"}, {"deja-dup-monitor", "deja-dup"}
        });
        auto procPos = procNameCorrections.find(procName);
        if (procPos != procNameCorrections.end()) {
            procName = procPos->second;
        }

        QDirIterator dir("/usr/share/applications", QDirIterator::Subdirectories);
        QString desktopFile;
        QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");
        while(dir.hasNext()) {
            if (dir.fileInfo().suffix() == "desktop") {
                if (dir.fileName().toLower().contains(procName.toLower())) {
                    desktopFile = dir.filePath();
                    break;
                }
            }
            dir.next();
        }

        if (desktopFile.size() == 0) {
            return defaultExecutableIcon;
        }

        QIcon icon = defaultExecutableIcon;
        QString iconName;
        QFile in(desktopFile);
        in.open(QIODevice::ReadOnly);
        while(!in.atEnd()) {
            iconName = in.readLine().trimmed();
            if (iconName.startsWith("Icon=")) {
                iconName.remove(0,5); 
            } else {
                continue;
            }

            if (iconName.contains("/")) {
                icon = QIcon(iconName);
            } else {
                icon = QIcon::fromTheme(iconName,defaultExecutableIcon);
                break;
            }
        }
        in.close();

        processIconMapCache[procName] = icon;
        return icon;
    }
}
