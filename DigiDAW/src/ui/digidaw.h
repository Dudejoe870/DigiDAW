#pragma once

#include "ui/common.h"

namespace DigiDAW::UI
{
    class DigiDAW : public sciter::om::asset<DigiDAW>
    {
    public:
        DigiDAW() {}

        SOM_PASSPORT_BEGIN(DigiDAW)
        SOM_PASSPORT_END
    };
}
