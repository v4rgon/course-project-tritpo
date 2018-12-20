#ifndef RESOURCESWORKER_H
#define RESOURCESWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QProgressBar>
#include <QLabel>
#include "qcustomplot.h"
#include <vector>
#include <QVector>
#include "cputools.h"
#include <deque>
#include <QSettings>
#include "memoryconverter.h"
#include <QHash>

typedef QVector<QVector<double>> qcustomplotCpuVector;

struct struct__intArrayHolder {
    int array[3];
};

class resourcesWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit resourcesWorker(QObject *parent, QSettings *settings);
    const QHash<QString, struct__intArrayHolder> getColourDefaults() {
        return defaultColours;
    }

    ~resourcesWorker();
signals:
    void updateMemoryBar(int value);
    void updateMemoryText(QString value);
    void updateSwapBar(int value);
    void updateSwapText(QString value);
    void updateCpuPlotSIG(const qcustomplotCpuVector &values);
private:
    void loop();
    QProgressBar *memoryBar, *swapBar;
    QLabel *memoryLabel, *swapLabel;
    void updateMemory();
    void updateSwap();
    void updateCpu();
    std::vector<cpuTools::cpuStruct> prevCpuTimes;
    std::deque<std::vector<double>> cpuPlotData;
    QSettings *settings;
    unitStandard standard;
    QHash<QString, struct__intArrayHolder> defaultColours;
public slots:
    void createColourDialogue();
};

#endif // RESOURCESWORKER_H
