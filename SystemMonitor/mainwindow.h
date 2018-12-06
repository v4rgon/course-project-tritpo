#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "processinformationworker.h"
#include "resourcesworker.h"
#include <QAction>
#include <QSettings>
#include <utility>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void handleTabChange();

private:
    processInformationWorker* processesThread;
    resourcesWorker* resourcesThread;
    QTabWidget* mainTabs;
    QAction *quitAction;
    QSettings *settings;
    void updateCpuAreaInfo(const QVector<double> &input);
};

#endif // MAINWINDOW_H
