#include "sciter-x.h"
#include "sciter-x-window.hpp"

#include "RtAudio.h"

class GUIFrame : public sciter::window 
{
private:
    RtAudio& dac;
public:
    GUIFrame(RtAudio& dac) : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG), dac(dac)
    {
    }

    SOM_PASSPORT_BEGIN(GUIFrame)
        SOM_FUNCS(
            SOM_FUNC(getDeviceCount)
        )
    SOM_PASSPORT_END

    unsigned int getDeviceCount()
    {
        return dac.getDeviceCount();
    }
};

#include "resources.cpp"

int uimain(std::function<int()> run) 
{
    RtAudio dac(RtAudio::Api::UNSPECIFIED);

    sciter::archive::instance().open(aux::elements_of(resources));

    sciter::om::hasset<GUIFrame> pwin = new GUIFrame(dac);
    SciterSetOption(NULL, SCITER_SET_UX_THEMING, TRUE);

    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand(true);

    return run();
}
