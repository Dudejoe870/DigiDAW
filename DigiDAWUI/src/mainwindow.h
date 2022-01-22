#pragma once

#include <QMainWindow>
#include <QCloseEvent>

#include "settings.h"
#include "about.h"

#include "main.h"

namespace Ui 
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(MainApplication* mainApp, QWidget* parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent* event);
private slots:
    void on_actionExit_triggered();
    void on_actionSettings_triggered();
    void on_actionAbout_triggered();
private:
    MainApplication* mainApp;

    Ui::MainWindow* ui;
    Settings* settingsWindow = nullptr;
    About* aboutWindow = nullptr;
};
