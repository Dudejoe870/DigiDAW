#include "ui/common.h"

#include "ui/mainwindow.h"

#include "main.h"

using namespace DigiDAW::UI;

int uimain(std::function<int()> run) 
{
    DigiDAW::MainApplication app;

    sciter::om::hasset<MainWindow> pwin = new MainWindow();

    pwin->load(WSTR("this://app/main.htm"));
    pwin->expand(true);

    return run();
}
