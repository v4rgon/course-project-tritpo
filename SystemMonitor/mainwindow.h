#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "processinformationworker.h"
#include "resourcesworker.h"
#include <QAction>
#include "qcustomplot.h"
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

public slots:
    void updateCpuPlotSLO(const qcustomplotCpuVector &input);

private slots:
    void handleTabChange();

private:
    processInformationWorker* processesThread;
    resourcesWorker* resourcesThread;
    QTabWidget* mainTabs;
    QAction *quitAction;
    QCustomPlot *cpuPlot;
    QSettings *settings;
    QGridLayout *cpuInfoArea;
    void updateCpuAreaInfo(const QVector<double> &input);
    QHash<QString, struct__intArrayHolder> defaultCpuColours;
    QPair<QVector<QVector<double>>, qcustomplotCpuVector> generateSpline(QString name, QVector<double> &x, const QVector<QVector<double>> &y, bool setMax = false);
};

#endif // MAINWINDOW_H
