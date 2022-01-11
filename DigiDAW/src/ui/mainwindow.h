#pragma once

#include "ui/common.h"

namespace DigiDAW::UI
{
    class MainWindow : public sciter::window
    {
    private:
    public:
        MainWindow() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG)
        {
        }

        SOM_PASSPORT_BEGIN(MainWindow)
            SOM_PASSPORT_END
    };
}
