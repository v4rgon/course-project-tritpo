#include "processinformationworker.h"
#include <QTabWidget>
#include <pwd.h>
#include "tablenumberitem.h"
#include <signal.h>
#include <QMessageBox>
#include <QHeaderView>
#include "processtools.h"
#include <proc/sysinfo.h>
#include <QAction>
using namespace processTools;

processInformationWorker::processInformationWorker(QObject *parent, QSettings *settings) :
    QObject(parent), workerThread() {
    mainWindow = parent;
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");

    loadAverage = parent->findChild<QLabel*>("loadAvgLabel");
    connect(this,SIGNAL(updateLoadAverage(QString)),loadAverage,SLOT(setText(QString)));

    totalCpuTime = 0;
    selectedRowInfoID = 0;

    QAction* actionStop = new QAction("Stop",processesTable);
    connect(actionStop,SIGNAL(triggered(bool)),SLOT(handleProcessStop()));

    QAction* actionKill = new QAction("Kill",processesTable);
    connect(actionKill, SIGNAL(triggered(bool)),SLOT(handleProcessKill()));

    QList<QAction*> rightClickActions;
    rightClickActions.push_back(actionStop);
    rightClickActions.push_back(actionKill);

    processesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    processesTable->addActions(rightClickActions);

    connect(this,SIGNAL(signalFilterProcesses(QString)),this,SLOT(filterProcesses(QString)));
    connect(processesTable->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this,SLOT(changeCurrentTableRowSelection(QModelIndex)));

    connect(this,SIGNAL(updateTableData()),SLOT(updateTable()));
    createProcessesView();

    this->settings = settings;

}





void processInformationWorker::changeCurrentTableRowSelection(QModelIndex current)
{
    selectedRowInfoID = processesTable->item(current.row(),3)->text().toInt();
}


void processInformationWorker::createProcessesView()
{
    processesTable->setColumnCount(4);
    processesTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    processesTable->setHorizontalHeaderLabels(QString("Process Name;User;% CPU;PID;").split(";"));
    processesTable->verticalHeader()->setVisible(false);
    processesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processesTable->resizeColumnsToContents();
}


void processInformationWorker::handleProcessStop()
{
    int row = processesTable->currentIndex().row();
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to stop "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(processesTable, "Info", QString::fromStdString(info),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (kill(pid,SIGTERM)!=0) {
            if (errno==EPERM) {
                QMessageBox::information(processesTable, "Error",
                                "Could not send signal, permission denied!", QMessageBox::Ok);
            }
        }
    }
}

/**
 * @brief processInformationWorker::handleProcessKill Function to run when the user selects stop as an option for a process
 */
void processInformationWorker::handleProcessKill()
{
    /// TODO: ask for privilage escalation when controlling other users' processes
    int row = processesTable->currentIndex().row();
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to kill "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(processesTable, "Info", QString::fromStdString(info),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (kill(pid,SIGKILL)!=0) {
            if (errno==EPERM) {
                QMessageBox::information(processesTable, "Error",
                                "Could not send signal, permission denied!", QMessageBox::Ok);
            }
        }
    }
}




void processInformationWorker::updateTable() {
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLCOM);
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));
    storedProcType processes;
    while (readproc(proc, &proc_info) != NULL) {
        processes[proc_info.tid]=proc_info;
    }
    closeproc(proc);
    if (prevProcs.size()>0) {
        for(auto &newItr:processes) {
            auto prevItr = prevProcs[newItr.first];
            newItr.second.pcpu = (unsigned int)calculateCPUPercentage(&prevItr, &newItr.second, totalCpuTime);
        }
    }
    totalCpuTime = getTotalCpuTime();

    processesTable->setUpdatesEnabled(false);
    processesTable->setSortingEnabled(false);
    processesTable->setRowCount(processes.size());
    unsigned int index = 0;
    for(auto &i:processes) {
        proc_t* p = (&i.second);
        QString processName = getProcessName(p);
        QTableWidgetItem* processNameTableItem = new QTableWidgetItem(processName);
        processNameTableItem->setToolTip(getProcessCmdline(p->tid));
        processNameTableItem->setIcon(getProcessIconFromName(processName,processIconCache));
        processesTable->setItem(index,0,processNameTableItem);
        QString user = p->euser;
        processesTable->setItem(index,1,new QTableWidgetItem(user));
        QString cpu = QString::number(settings->value("divide process cpu by cpu count",false).toBool()? p->pcpu/smp_num_cpus:p->pcpu);
        processesTable->setItem(index,2,new TableNumberItem(cpu));
        QString id = QString::number(p->tid);
        processesTable->showRow(index);


        if (selectedRowInfoID>0) {
            if (selectedRowInfoID == p->tid) {
                processesTable->selectRow(index);
            }
        }

        index++;
    }

    processesTable->setUpdatesEnabled(true);
    processesTable->setSortingEnabled(true);
    prevProcs = processes;
}


void processInformationWorker::loop()
{
    emit(updateTableData());
    double load[3];
    getloadavg(load,3);
    double overload = load[0] - smp_num_cpus;
    QString avg = "Load averages for the last 1, 5, 15 minutes: " + QString::number(load[0])
            + ", " + QString::number(load[1]) + ", " + QString::number(load[2])
            + (overload<0? "":"\nOverloaded by " + QString::number(overload*100) + "% in the last minute!");
    emit(updateLoadAverage(avg));
    if (settings->value("processes update interval",1.0).toDouble() < 0.25) {
        settings->setValue("processes update interval",0.25);
    }
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(
                                    settings->value("processes update interval",1.0).toDouble() * 1000));
}

