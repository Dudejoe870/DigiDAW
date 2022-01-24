#pragma once

#include <QWidget>
#include <QShowEvent>
#include <QAbstractButton>
#include <QTimer>

#include "main.h"

namespace Ui
{
    class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT
public:
    explicit Settings(MainApplication* mainApp, QWidget* parent = 0);
    ~Settings();

    void updateControls();

    void applySettings();

    void updateDevices();

    void updateDeviceDependantControls();

    void showEvent(QShowEvent* event);
private slots:
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_test_button_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_api_combo_currentIndexChanged(int index);
    void on_output_combo_currentIndexChanged(int index);
    void on_input_combo_currentIndexChanged(int index);

    void stopTestTone();
private:
    bool updatingControls = false;

    MainApplication* mainApp;

    Ui::Settings* ui;

    QTimer* testToneTimer = nullptr;

    RtAudio::Api getCurrentSelectedApi();
    int getCurrentSelectedBufferSize();
    int getCurrentSelectedOutputDevice();
    int getCurrentSelectedInputDevice();
    int getCurrentSelectedSampleRate();
};
