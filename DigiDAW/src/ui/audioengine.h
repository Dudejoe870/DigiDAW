#pragma once

#include "ui/common.h"

#include "main.h"

namespace DigiDAW::UI
{
    class AudioEngine : public sciter::om::asset<AudioEngine>
    {
    private:
        MainApplication* pApp;

        sciter::value audioFormats;
        int returnCode;

        bool setOutputDevice(int device)
        {
            returnCode = (int)pApp->audioEngine->setCurrentOutputDevice((unsigned int)device);
            return true;
        }

        int getOutputDevice()
        {
            return (int)pApp->audioEngine->getCurrentOutputDevice();
        }

        bool setInputDevice(int device)
        {
            returnCode = (int)pApp->audioEngine->setCurrentInputDevice((unsigned int)device);
            return true;
        }

        int getInputDevice()
        {
            return (int)pApp->audioEngine->getCurrentInputDevice();
        }

        bool setSampleRate(int rate)
        {
            returnCode = (int)pApp->audioEngine->setCurrentSampleRate((unsigned int)rate);
            return true;
        }

        int getSampleRate()
        {
            return (int)pApp->audioEngine->getCurrentSampleRate();
        }

        bool setBufferSize(int size)
        {
            returnCode = (int)pApp->audioEngine->setCurrentBufferSize((unsigned int)size);
            return true;
        }

        int getBufferSize()
        {
            return (int)pApp->audioEngine->getCurrentBufferSize();
        }

        bool getIsStreamRunning()
        {
            return pApp->audioEngine->isStreamRunning();
        }

        bool getIsStreamOpen()
        {
            return pApp->audioEngine->isStreamOpen();
        }
    public:
        AudioEngine(MainApplication* pApp);

        sciter::astring getAPIDisplayName(int api);

        std::vector<int> getSupportedAPIs();
        int getCurrentAPI();

        void changeBackend(int api);

        std::vector<sciter::value> queryDevices();

        std::vector<int> getSupportedSampleRates();

        void start();
        void stop();
        void pause();

        SOM_PASSPORT_BEGIN(AudioEngine)
            SOM_FUNCS(
                SOM_FUNC(getAPIDisplayName), 
                SOM_FUNC(getSupportedAPIs), 
                SOM_FUNC(getCurrentAPI),
                SOM_FUNC(changeBackend),
                SOM_FUNC(queryDevices),
                SOM_FUNC(getSupportedSampleRates),
                SOM_FUNC(start),
                SOM_FUNC(stop),
                SOM_FUNC(pause))
            SOM_PROPS(
                SOM_RO_PROP(audioFormats),
                SOM_RO_PROP(returnCode),
                SOM_VIRTUAL_PROP(outputDevice, getOutputDevice, setOutputDevice),
                SOM_VIRTUAL_PROP(inputDevice, getInputDevice, setInputDevice),
                SOM_VIRTUAL_PROP(sampleRate, getSampleRate, setSampleRate),
                SOM_VIRTUAL_PROP(bufferSize, getBufferSize, setBufferSize),
                SOM_RO_VIRTUAL_PROP(isStreamRunning, getIsStreamRunning),
                SOM_RO_VIRTUAL_PROP(isStreamOpen, getIsStreamOpen))
        SOM_PASSPORT_END
    };
}
