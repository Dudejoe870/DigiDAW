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

        bool setOutputDevice(int device)
        {
            pApp->audioEngine->setCurrentOutputDevice((unsigned int)device);
            return true;
        }

        int getOutputDevice()
        {
            return (int)pApp->audioEngine->getCurrentOutputDevice();
        }

        bool setInputDevice(int device)
        {
            pApp->audioEngine->setCurrentInputDevice((unsigned int)device);
            return true;
        }

        int getInputDevice()
        {
            return (int)pApp->audioEngine->getCurrentInputDevice();
        }

        bool setSampleRate(int rate)
        {
            pApp->audioEngine->setCurrentSampleRate((unsigned int)rate);
            return true;
        }

        int getSampleRate()
        {
            return (int)pApp->audioEngine->getCurrentSampleRate();
        }
    public:
        AudioEngine(MainApplication* pApp);

        sciter::astring getAPIDisplayName(int api);

        std::vector<int> getSupportedAPIs();
        int getCurrentAPI();

        void changeBackend(int api);

        std::vector<sciter::value> queryDevices();

        std::vector<int> getSupportedSampleRates();

        SOM_PASSPORT_BEGIN(AudioEngine)
            SOM_FUNCS(
                SOM_FUNC(getAPIDisplayName), 
                SOM_FUNC(getSupportedAPIs), 
                SOM_FUNC(getCurrentAPI),
                SOM_FUNC(changeBackend),
                SOM_FUNC(queryDevices),
                SOM_FUNC(getSupportedSampleRates))
            SOM_PROPS(
                SOM_RO_PROP(audioFormats),
                SOM_VIRTUAL_PROP(outputDevice, getOutputDevice, setOutputDevice),
                SOM_VIRTUAL_PROP(inputDevice, getInputDevice, setInputDevice),
                SOM_VIRTUAL_PROP(sampleRate, getSampleRate, setSampleRate))
        SOM_PASSPORT_END
    };
}
