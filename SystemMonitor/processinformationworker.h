#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <proc/readproc.h>
#include <map>
#include "workerthread.h"
#include <QTableWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <string>
#include <vector>
#include <QLabel>
#include <unordered_map>
#include <QSettings>
#include "hashqstring.h"

class processInformationWorker : public QObject, public workerThread
{
    typedef std::map<int, proc_t> storedProcType;
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent, QSettings *settings);

private slots:
    void handleProcessStop();
    void handleProcessKill();
    void updateTable();
    void filterProcesses(QString filter);
    void changeCurrentTableRowSelection(QModelIndex current);
    void showProcessProperties();
    void filterCheckboxToggled(bool checked);

signals:
    void updateTableData();
    void signalFilterProcesses(QString filter);
    void updateLoadAverage(QString value);

private:
    void loop();
    QObject* mainWindow;
    QTableWidget* processesTable;
    void createProcessesView();
    QCheckBox* filterCheckbox;
    QLineEdit* searchField;
    bool shouldHideProcess(unsigned int pid);
    storedProcType prevProcs;
    unsigned long long totalCpuTime;
    int selectedRowInfoID;
    QLabel* loadAverage;
    std::unordered_map<QString, QIcon> processIconCache;
    QSettings *settings;
};

#endif // PROCESSINFORMATIONWORKER_H
