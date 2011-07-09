#pragma once
#include <OgreVector3.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>

// Forward Declarations
#include <OgrePrerequisites.h>
namespace OIS {
  class InputManager;
  class Mouse;
  class Keyboard;
};
class Level;

class Application: public Ogre::FrameListener, public Ogre::WindowEventListener
{
public:
  Application();
  virtual ~Application();

  virtual void go(void);

protected:
  virtual bool setup(void);
  virtual bool configure(void);
  virtual void chooseSceneManager(void);
  virtual void createCamera(void);
  virtual void createFrameListener(void);
  virtual void createScene(void);
  virtual void destroyScene(void) {}    // Optional to override this
  virtual void createViewports(void);
  virtual void setupResources(void);
  virtual void createResourceListener(void);
  virtual void loadResources(void);

  // FrameListener
  void updateStats();
  void showDebugOverlay(bool show);
  bool handleKeyboardInput(const Ogre::FrameEvent& evt);
  bool handleMouseInput(const Ogre::FrameEvent& evt);
  bool frameEnded(const Ogre::FrameEvent& evt); // Inherited
  bool frameStarted(const Ogre::FrameEvent& evt); // Inherited

  // WindowEventListener
  virtual void windowResized(Ogre::RenderWindow* rw); // Inherited
  virtual void windowClosed(Ogre::RenderWindow* rw); // Inherited

protected:
  Ogre::Root *mpRoot;
  Ogre::Camera* mpCamera;
  Ogre::SceneNode* mpCamNode;
  Ogre::SceneManager* mpSceneMgr;
  Ogre::RenderWindow* mpWindow;
  Ogre::String mResourcePath;
  Ogre::String mDebugText;
  Level* mpLevel;

  // Input Related Stuff
	Ogre::Overlay* mpDebugOverlay;
  Ogre::Real mTimeUntilNextToggle;
  bool mStatsOn;
  int mSceneDetailIndex;

  // OIS Input devices
	OIS::InputManager*  mpInputManager;
	OIS::Mouse*         mpMouse;
	OIS::Keyboard*      mpKeyboard;
};