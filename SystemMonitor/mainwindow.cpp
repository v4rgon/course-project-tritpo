#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

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
