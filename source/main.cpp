#include "Application.h"
#include <time.h>
#include <OgreException.h>

#define WIN32_LEAN_AND_MEAN
#include "windows.h"


//INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
int main(int argc, char **argv)
{
    // Create application object
    Application app;

    try {
        app.go();
    } catch(Ogre::Exception& e) {
        MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
    } catch(std::exception& e) {
      MessageBox(NULL, e.what(), "Uh oh!", MB_OK | MB_ICONERROR);
    } catch(...) {
      MessageBox(NULL, "Unknown exception thrown!", "Uh oh!", MB_OK | MB_ICONERROR);
    }

    return 0;
}
