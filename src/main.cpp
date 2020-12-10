#include "global.hpp"
#include "app_interface.hpp"
#include <SDL.h> // main -> SDL_main redefinition.

// @warning SDL 2 requires this function signature to avoid SDL_main linker errors.
int main(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    IApp* a = IApp::Get();

    try
    {
        if (a->Init())
            ret = a->Loop();
    }
    catch (const std::exception& e)
    {
        LogFatal("%s.", e.what());
    }

    a->Cleanup();
    return ret;
}
