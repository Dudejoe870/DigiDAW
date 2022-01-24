#include "settings.h"
#include "ui_settings.h"

#include "audio/engine.h"

Settings::Settings(MainApplication* mainApp, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    this->mainApp = mainApp;

    ui->setupUi(this);

    this->testToneTimer = new QTimer(this);
    connect(testToneTimer, &QTimer::timeout, this, &Settings::stopTestTone);
}

void Settings::updateControls()
{
    if (!updatingControls)
    {
        updatingControls = true;

        this->setCursor(QCursor(Qt::WaitCursor));

        ui->api_combo->clear();
        std::vector<RtAudio::Api> supportedAPIs = mainApp->audioEngine->getSupportedAPIs();
        for (RtAudio::Api api : supportedAPIs)
        {
            ui->api_combo->addItem(QString(mainApp->audioEngine->getAPIDisplayName(api).c_str()), api);
            if (api == mainApp->audioEngine->getCurrentAPI())
                ui->api_combo->setCurrentIndex(ui->api_combo->count() - 1);
        }

        ui->buffersize_combo->clear();
        for (unsigned int buffersize = 32; buffersize <= 4096; buffersize *= 2)
        {
            ui->buffersize_combo->addItem(QString::number(buffersize) + " Samples", buffersize);
            if (buffersize == mainApp->audioEngine->getCurrentBufferSize())
                ui->buffersize_combo->setCurrentIndex(ui->buffersize_combo->count() - 1);
        }

        this->unsetCursor();

        updatingControls = false;
    }
}

void Settings::applySettings()
{
    this->setCursor(QCursor(Qt::WaitCursor));

    unsigned int currentOutputDevice = (unsigned int)getCurrentSelectedOutputDevice();
    if (mainApp->audioEngine->getCurrentOutputDevice() != currentOutputDevice)
        mainApp->audioEngine->setCurrentOutputDevice(currentOutputDevice);

    unsigned int currentInputDevice = (unsigned int)getCurrentSelectedInputDevice();
    if (mainApp->audioEngine->getCurrentInputDevice() != currentInputDevice)
        mainApp->audioEngine->setCurrentInputDevice(currentInputDevice);

    unsigned int currentSampleRate = (unsigned int)getCurrentSelectedSampleRate();
    if (mainApp->audioEngine->getCurrentSampleRate() != currentSampleRate)
        mainApp->audioEngine->setCurrentSampleRate(currentSampleRate);

    unsigned int currentBufferSize = (unsigned int)getCurrentSelectedBufferSize();
    if (mainApp->audioEngine->getCurrentBufferSize() != currentBufferSize)
        mainApp->audioEngine->setCurrentBufferSize(currentBufferSize);

    this->unsetCursor();
}

void Settings::updateDevices()
{
    std::vector<DigiDAW::Audio::Engine::AudioDevice> devices = mainApp->audioEngine->getDevices();
    RtAudio::Api currentApi = mainApp->audioEngine->getCurrentAPI();
    RtAudio::Api selectedApi = getCurrentSelectedApi();

    ui->output_combo->clear();
    ui->output_combo->addItem("None", -1);
    ui->output_combo->setCurrentIndex(0);

    ui->input_combo->clear();
    ui->input_combo->addItem("None", -1);
    ui->input_combo->setCurrentIndex(0);

    for (DigiDAW::Audio::Engine::AudioDevice device : devices)
    {
        if (device.info.probed)
        {
            if (device.info.outputChannels > 0)
            {
                ui->output_combo->addItem(device.info.name.c_str(), device.index);

                if (currentApi == selectedApi && mainApp->audioEngine->getCurrentOutputDevice() == device.index)
                    ui->output_combo->setCurrentIndex(ui->output_combo->count() - 1);
            }

            if (device.info.inputChannels > 0)
            {
                ui->input_combo->addItem(device.info.name.c_str(), device.index);

                if (currentApi == selectedApi && mainApp->audioEngine->getCurrentInputDevice() == device.index)
                    ui->input_combo->setCurrentIndex(ui->input_combo->count() - 1);
            }
        }
    }
}

void Settings::updateDeviceDependantControls()
{
    unsigned int currentOutputDevice = getCurrentSelectedOutputDevice();
    unsigned int currentInputDevice = getCurrentSelectedInputDevice();

    std::vector<DigiDAW::Audio::Engine::AudioDevice> devices = mainApp->audioEngine->getDevices();

    std::vector<unsigned int> supportedSampleRates;
    mainApp->audioEngine->getSupportedSampleRates(supportedSampleRates, currentOutputDevice, currentInputDevice);

    ui->samplerate_combo->clear();
    bool containsCurrentSampleRate = false;
    for (unsigned int sampleRate : supportedSampleRates)
    {
        ui->samplerate_combo->addItem(QString::number(sampleRate) + "hz", sampleRate);
        if (mainApp->audioEngine->getCurrentSampleRate() == sampleRate)
        {
            ui->samplerate_combo->setCurrentIndex(ui->samplerate_combo->count() - 1);
            containsCurrentSampleRate = true;
        }
    }
    if (!containsCurrentSampleRate) ui->samplerate_combo->setCurrentIndex(ui->samplerate_combo->count() - 1);

    if (currentOutputDevice != -1)
    {
        DigiDAW::Audio::Engine::AudioDevice& device = devices[currentOutputDevice];
        ui->output_combo->setToolTip(
            "Output Channels: " + QString::number(device.info.outputChannels) + 
            "\nInput Channels: " + QString::number(device.info.inputChannels) +
            "\nPreferred Sample Rate: " + QString::number(device.info.preferredSampleRate));
    }
    else ui->output_combo->setToolTip("None");

    if (currentInputDevice != -1)
    {
        DigiDAW::Audio::Engine::AudioDevice& device = devices[currentInputDevice];
        ui->input_combo->setToolTip(
            "Output Channels: " + QString::number(device.info.outputChannels) +
            "\nInput Channels: " + QString::number(device.info.inputChannels) +
            "\nPreferred Sample Rate: " + QString::number(device.info.preferredSampleRate));
    }
    else ui->input_combo->setToolTip("None");
}

void Settings::showEvent(QShowEvent* event)
{
    updateControls();
    updateDevices();
}

void Settings::on_buttonBox_clicked(QAbstractButton* button)
{
    if (button)
    {
        QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);

        switch (role)
        {
        case QDialogButtonBox::ButtonRole::ApplyRole: // Apply
            applySettings();
            updateControls();
            break;
        default:
            break;
        }
    }
}

void Settings::on_test_button_clicked()
{
    mainApp->audioEngine->startEngine();
    mainApp->audioEngine->mixer.startTestTone();
    testToneTimer->start(1000);
}

void Settings::on_tabWidget_currentChanged(int index)
{
    updateControls();
    updateDevices();
}

void Settings::on_api_combo_currentIndexChanged(int index)
{
    if (!updatingControls)
    {
        RtAudio::Api currentApi = getCurrentSelectedApi();
        if (mainApp->audioEngine->getCurrentAPI() != currentApi)
            mainApp->audioEngine->changeBackend(currentApi);

        updateDevices();
    }
}

void Settings::on_output_combo_currentIndexChanged(int index)
{
    updateDeviceDependantControls();
}

void Settings::on_input_combo_currentIndexChanged(int index)
{
    updateDeviceDependantControls();
}

void Settings::stopTestTone()
{
    mainApp->audioEngine->mixer.endTestTone();
    mainApp->audioEngine->stopEngine();
}

RtAudio::Api Settings::getCurrentSelectedApi()
{
    return static_cast<RtAudio::Api>(ui->api_combo->itemData(ui->api_combo->currentIndex()).toInt());
}

int Settings::getCurrentSelectedBufferSize()
{
    return ui->buffersize_combo->itemData(ui->buffersize_combo->currentIndex()).toInt();
}

int Settings::getCurrentSelectedOutputDevice()
{
    return ui->output_combo->itemData(ui->output_combo->currentIndex()).toInt();
}

int Settings::getCurrentSelectedInputDevice()
{
    return ui->input_combo->itemData(ui->input_combo->currentIndex()).toInt();
}

int Settings::getCurrentSelectedSampleRate()
{
    return ui->samplerate_combo->itemData(ui->samplerate_combo->currentIndex()).toInt();
}

Settings::~Settings()
{
    delete ui;
    if (testToneTimer) delete testToneTimer;
}
