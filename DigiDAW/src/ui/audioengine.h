#pragma once

#include "ui/common.h"

#include "main.h"

namespace DigiDAW::UI
{
    class AudioEngine : public sciter::om::asset<AudioEngine>
    {
    private:
        MainApplication* pApp;

        std::vector<sciter::astring> apiMapping;
    public:
        AudioEngine(MainApplication* pApp);

        std::vector<sciter::astring> getAPIEnum();
        sciter::astring getAPIDisplayName(int api);

        std::vector<int> getSupportedAPIs();
        int getCurrentAPI();

        void changeBackend(int api);

        std::vector<sciter::value> queryDevices();

        SOM_PASSPORT_BEGIN(AudioEngine)
            SOM_FUNCS(
                SOM_FUNC(getAPIEnum), 
                SOM_FUNC(getAPIDisplayName), 
                SOM_FUNC(getSupportedAPIs), 
                SOM_FUNC(getCurrentAPI),
                SOM_FUNC(changeBackend),
                SOM_FUNC(queryDevices))
        SOM_PASSPORT_END
    };
}
