#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settings.h"

MainWindow::MainWindow(MainApplication* mainApp, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->mainApp = mainApp;

    ui->setupUi(this);
    this->showMaximized();

    settingsWindow = new Settings(mainApp);
    aboutWindow = new About();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (settingsWindow) delete settingsWindow;
    if (aboutWindow) delete aboutWindow;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    settingsWindow->close();
    aboutWindow->close();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionSettings_triggered()
{
    settingsWindow->show();
}

void MainWindow::on_actionAbout_triggered()
{
    aboutWindow->show();
}
