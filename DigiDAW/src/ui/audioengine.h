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
            unsigned int ret;
            pApp->audioEngine->getCurrentOutputDevice(ret);
            return (int)ret;
        }

        bool setInputDevice(int device)
        {
            pApp->audioEngine->setCurrentInputDevice((unsigned int)device);
            return true;
        }

        int getInputDevice()
        {
            unsigned int ret;
            pApp->audioEngine->getCurrentInputDevice(ret);
            return (int)ret;
        }

        bool setSampleRate(int rate)
        {
            pApp->audioEngine->setCurrentSampleRate((unsigned int)rate);
            return true;
        }

        int getSampleRate()
        {
            unsigned int ret;
            pApp->audioEngine->getCurrentSampleRate(ret);
            return (int)ret;
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
