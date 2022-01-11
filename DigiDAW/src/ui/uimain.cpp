#include "ui/common.h"

#include "ui/mainwindow.h"
#include "ui/registry.h"

#include "ui/resources.cpp"

using namespace DigiDAW::UI;

int uimain(std::function<int()> run) 
{
    sciter::archive::instance().open(aux::elements_of(resources));

    sciter::om::hasset<MainWindow> pwin = new MainWindow();
    
    Registry::RegisterAllAssets();

    pwin->load(WSTR("this://app/main.htm"));
    pwin->expand(true);

    return run();
}
