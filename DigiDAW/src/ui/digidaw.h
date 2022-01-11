#pragma once

#include "ui/common.h"

namespace DigiDAW::UI
{
    class DigiDAW : public sciter::om::asset<DigiDAW>
    {
    public:
        DigiDAW() {}

        sciter::astring getStatusMessage();

        SOM_PASSPORT_BEGIN(DigiDAW)
            SOM_FUNCS(SOM_FUNC(getStatusMessage))
        SOM_PASSPORT_END
    };
}
