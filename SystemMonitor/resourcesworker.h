#ifndef RESOURCESWORKER_H
#define RESOURCESWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QProgressBar>
#include <QLabel>
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

    ~resourcesWorker();
private:

    QSettings *settings;

};

#endif // RESOURCESWORKER_H
