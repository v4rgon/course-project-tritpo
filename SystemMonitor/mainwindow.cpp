#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "colourhelper.h"
using namespace colourHelper;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("system-monitor","system-monitor");

    QDataStream geometry(settings->value("mainWindowGeometry", this->geometry()).toByteArray());
    QRect geomRect;
    geometry >> geomRect;
    this->setGeometry(geomRect);

    processesThread = new processInformationWorker(this, settings);
    resourcesThread = new resourcesWorker(this, settings);

    processesThread->start();
    resourcesThread->start();
    quitAction = this->findChild<QAction*>("actionQuit");
    connect(quitAction,SIGNAL(triggered(bool)),QApplication::instance(),SLOT(quit()));

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    qRegisterMetaType<qcustomplotCpuVector>("qcustomplotCpuVector");
    cpuPlot = reinterpret_cast<QCustomPlot*>(ui->tabResources->findChild<QWidget*>("cpuPlot"));
    connect(resourcesThread,SIGNAL(updateCpuPlotSIG(const qcustomplotCpuVector&)),
            this,SLOT(updateCpuPlotSLO(const qcustomplotCpuVector&)));
    cpuInfoArea = ui->tabResources->findChild<QGridLayout*>("cpuInfoArea");

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete processesThread;
    delete resourcesThread;
    delete mainTabs;
    delete settings;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);

   QByteArray data;
   QDataStream geometry(&data, QIODevice::WriteOnly);
   geometry << this->geometry();
   settings->setValue("mainWindowGeometry", data);
}

void MainWindow::updateCpuAreaInfo(const QVector<double> &input)
{
    static QVector<QLabel*> cpuLabels;
    static QVector<QPushButton*> cpuColourButtons;
    #define colourNamesLen 4
    static const QString colourNames[] = {
        "orange","red","green","blue"
    };

    if (cpuInfoArea->count() == 0) {
        for(int i=0; i<input.size(); i++) {
            QLabel *cpuLabel = new QLabel(QString::number(input.at(i)));
            cpuLabels.append(cpuLabel);

            QPushButton *cpuColourButton = new QPushButton();
            cpuColourButton->setFlat(true);
            cpuColourButton->setObjectName("CPU" + QString::number(i+1));
            cpuColourButtons.append(cpuColourButton);
            connect(cpuColourButton, SIGNAL(clicked(bool)), this->resourcesThread, SLOT(createColourDialogue()), Qt::UniqueConnection);

            QHBoxLayout *cpuLayout = new QHBoxLayout();
            cpuLayout->addWidget(cpuColourButton);
            cpuLayout->addWidget(cpuLabel);

            cpuInfoArea->addLayout(cpuLayout, i / 4, i % 4);
            cpuInfoArea->setAlignment(cpuLayout, Qt::AlignHCenter);

            QColor colour = QColor(colourNames[i % colourNamesLen]);
            colour = colour.toRgb();
            struct__intArrayHolder rgbColor;
            rgbColor.array[0] = colour.red();
            rgbColor.array[1] = colour.green();
            rgbColor.array[2] = colour.blue();
            this->defaultCpuColours["CPU" + QString::number(i+1)] = rgbColor;
        }
        return;
    }

    for(int i=0; i<cpuLabels.size(); i++) {
        cpuLabels[i]->setText("CPU" + QString::number(i+1) + " "
                                + QString::number(memoryConverter::roundDouble(input[i], 1)) + "%");

        QPixmap cpuColour = QPixmap(cpuColourButtons[i]->width(), cpuColourButtons[i]->height());
        cpuColour.fill(createColourFromSettings(settings, cpuColourButtons[i]->objectName(),
                                                      this->defaultCpuColours[cpuColourButtons[i]->objectName()].array));
        cpuColourButtons[i]->setIcon(QIcon(cpuColour));
    }
}

QPair<QVector<QVector<double>>, qcustomplotCpuVector> MainWindow::generateSpline(QString name, QVector<double> &x, const qcustomplotCpuVector &y, bool setMax)
{
    int size = y.size();
    QVector<QVector<double>> xs;
    for(unsigned int i=0; i < size; i++) {
        xs.push_back(x);
    }
    return QPair<QVector<QVector<double>>, qcustomplotCpuVector>(xs, y);
}

void MainWindow::updateCpuPlotSLO(const qcustomplotCpuVector &input)
{
    QVector<double> x(60);
    for (int i=59; i>0; --i)
    {
      x[i] = i;
    }

    const qcustomplotCpuVector *values = &input;

    static bool previouslyPlotted = false;
    int size = values->count();
    if (size == 0) {
        return;
    }

    QVector<QVector<double>> splineXValues;
    bool smooth = settings->value("smoothGraphs", false).toBool();
    QPair<QVector<QVector<double>>, qcustomplotCpuVector> data;
    if(smooth) {
        data = generateSpline("cpu", x, *values, true);
        if (!data.second.empty() && !data.second.at(0).empty()) {
            values = &data.second;
            x = data.first.at(0);
        }
        splineXValues = data.first;
    }

    QVector<double> lastValues;
    for(int i=0; i<size; i++) {
        lastValues.append(input.at(i).last());
    }
    updateCpuAreaInfo(lastValues);


    for(int i=0; i<size; i++) {
        QString colName = "CPU" + QString::number(i+1);
        QColor cpuColour = createColourFromSettings(settings, colName, this->defaultCpuColours[colName].array);

        if (!previouslyPlotted) {
            cpuPlot->addGraph();
        } else {
            cpuPlot->graph(i)->data()->clear();
            cpuPlot->graph(i)->setPen(QPen(QColor(cpuColour)));
        }

        cpuPlot->graph(i)->setData(x, values->at(i));

        if (settings->value("draw cpu area stacked",false).toBool()) {
            cpuPlot->graph(i)->setBrush(QBrush(cpuColour));
        } else {
            cpuPlot->graph(i)->setBrush(QBrush());
        }
    }
    previouslyPlotted = true;

    if (smooth) {
        cpuPlot->xAxis->setRange(0, x.last() + 1);
    } else {
        cpuPlot->xAxis->setRange(0, values->at(0).size());
    }
    cpuPlot->yAxis->setRange(0, 100);
    cpuPlot->replot();
}

void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    processesThread->setPaused(true);
    if (!settings->value("resourcesKeepThreadRunning", true).toBool()) {
        resourcesThread->setPaused(true);
    } else {
        resourcesThread->setPaused(false);
    }
    switch(index) {
        case 0:
            processesThread->setPaused(false);
        break;

        case 1:
            resourcesThread->setPaused(false);
        break;

    }
}
