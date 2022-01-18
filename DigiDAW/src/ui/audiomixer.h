#pragma once

#include "ui/common.h"

#include "main.h"

namespace DigiDAW::UI
{
    class AudioMixer : public sciter::om::asset<AudioMixer>
    {
    private:
        MainApplication* pApp;
    public:
        AudioMixer(MainApplication* pApp);

        void startTestTone();
        void endTestTone();

        SOM_PASSPORT_BEGIN(AudioMixer)
            SOM_FUNCS(
                SOM_FUNC(startTestTone),
                SOM_FUNC(endTestTone))
        SOM_PASSPORT_END
    };
}
