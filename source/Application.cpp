#include "Application.h"
#include <Ogre.h>
#include "Level.h"
#include "TowerFactory.h"
#include "common.h"
#include "gui.h"

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

Application::Application()
{
  mpRoot = 0;
  mpCamera = NULL;
  mpWindow = NULL;
  mStatsOn = false;
  mTimeUntilNextToggle = 0;
  mSceneDetailIndex = 0;
  mpDebugOverlay = 0;
  mpInputManager = NULL;
  mpMouse = NULL;
  mpKeyboard = NULL;
  mResourcePath = "";
  mpCurrentState = NULL;
}

/// Standard destructor
Application::~Application()
{
  if (mpRoot) delete mpRoot;

  //Remove ourself as a Window listener
  Ogre::WindowEventUtilities::removeWindowEventListener(mpWindow, this);
  windowClosed(mpWindow);
}

/// Start the example
void Application::go(void)
{
  if (!setup())
    return;

  mpRoot->startRendering();

  // clean up
  destroyScene();
}

bool Application::setup(void)
{
  Ogre::String pluginsPath;
  pluginsPath = mResourcePath + "plugins.cfg";


  mpRoot = new Ogre::Root(pluginsPath, mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");
  
  setupResources();

  bool carryOn = configure();
  if (!carryOn) return false;

  chooseSceneManager();
  createCamera();
  createViewports();

  // Set default mipmap level (NB some APIs ignore this)
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

  // Create any resource listeners (for loading screens)
  createResourceListener();
  // Load resources
  loadResources();

  // Create the frame listener
  createFrameListener();

  // Create the scene
  createScene();

  return true;

}

bool Application::configure(void)
{
  // Show the configuration dialog and initialise the system
  // You can skip this and use root.restoreConfig() to load configuration
  // settings if you were sure there are valid ones saved in ogre.cfg
  if(mpRoot->restoreConfig() || mpRoot->showConfigDialog())
  {
    // If returned true, user clicked OK so initialise
    // Here we choose to let the system create a default rendering window by passing 'true'
    mpWindow = mpRoot->initialise(true);
    
    return true;
  }
  else
  {
    return false;
  }
}

void Application::chooseSceneManager(void)
{
  // Create the SceneManager, in this case a generic one
  mpSceneMgr = mpRoot->createSceneManager(Ogre::ST_GENERIC, "DefaultSceneManager");
}

void Application::createCamera(void)
{
  // Create the camera
  mpCamera = mpSceneMgr->createCamera("PlayerCam");
  mpCamera->lookAt(Ogre::Vector3(0, -2, -3));
  mpCamera->setNearClipDistance(5);
  
  mpCamNode = mpSceneMgr->getRootSceneNode()->createChildSceneNode("CameraNode");
  mpCamNode->attachObject(mpCamera);
  mpCamNode->setPosition(0, 400, 0);
  mpCamNode->yaw(Ogre::Degree(-30));
}

void Application::createFrameListener(void)
{
  /** All of this is from the ExampleFrameListener constructor.  Since the Application
  is the FrameListener, I'm just going to do the same work here. */
  mpDebugOverlay = Ogre::OverlayManager::getSingleton().getByName("Core/DebugOverlay");

  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mpWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

  mpInputManager = OIS::InputManager::createInputSystem( pl );

  //Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
  mpKeyboard = static_cast<OIS::Keyboard*>(mpInputManager->createInputObject( OIS::OISKeyboard, false ));
  mpMouse = static_cast<OIS::Mouse*>(mpInputManager->createInputObject( OIS::OISMouse, false ));

  //Set initial mouse clipping size
  windowResized(mpWindow);

  showDebugOverlay(mStatsOn);

  //Register as a Window listener
  Ogre::WindowEventUtilities::addWindowEventListener(mpWindow, this);

  mpRoot->addFrameListener(this);
}

void Application::createViewports(void)
{
  // Create one viewport, entire window
  Ogre::Viewport* vp = mpWindow->addViewport(mpCamera);
  vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

  // Alter the camera aspect ratio to match the viewport
  mpCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

/// Method which will define the source of resources (other than current folder)
void Application::setupResources(void)
{
  // Load resource paths from config file
  Ogre::ConfigFile cf;
  cf.load(mResourcePath + "resources.cfg");

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
    secName = seci.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator i;
    for (i = settings->begin(); i != settings->end(); ++i)
    {
      typeName = i->first;
      archName = i->second;

      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
    }
  }
}

/// Optional override method where you can create resource listeners (e.g. for loading screens)
void Application::createResourceListener(void)
{

}

/// Optional override method where you can perform resource group loading
/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
void Application::loadResources(void)
{
  // Initialise, parse scripts etc
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Application::updateStats(void)
{
  static Ogre::String currFps = "Current FPS: ";
  static Ogre::String avgFps = "Average FPS: ";
  static Ogre::String bestFps = "Best FPS: ";
  static Ogre::String worstFps = "Worst FPS: ";
  static Ogre::String tris = "Triangle Count: ";
  static Ogre::String batches = "Batch Count: ";

  // update stats when necessary
  try {
    Ogre::OverlayElement* guiAvg = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
    Ogre::OverlayElement* guiCurr = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
    Ogre::OverlayElement* guiBest = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
    Ogre::OverlayElement* guiWorst = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

    const Ogre::RenderTarget::FrameStats& stats = mpWindow->getStatistics();
    guiAvg->setCaption(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
    guiCurr->setCaption(currFps + Ogre::StringConverter::toString(stats.lastFPS));
    guiBest->setCaption(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
      +" " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
    guiWorst->setCaption(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
      +" " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

    Ogre::OverlayElement* guiTris = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
    guiTris->setCaption(tris + Ogre::StringConverter::toString(stats.triangleCount));

    Ogre::OverlayElement* guiBatches = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
    guiBatches->setCaption(batches + Ogre::StringConverter::toString(stats.batchCount));

    Ogre::OverlayElement* guiDbg = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
    guiDbg->setCaption(mDebugText);
  }
  catch(...) { /* ignore */ }
}

void Application::showDebugOverlay(bool show)
{
  if (mpDebugOverlay)
  {
    if (show)
      mpDebugOverlay->show();
    else
      mpDebugOverlay->hide();
  }
}

//Adjust mouse clipping area
void Application::windowResized(Ogre::RenderWindow* rw)
{
  unsigned int width, height, depth;
  int left, top;
  rw->getMetrics(width, height, depth, left, top);

  const OIS::MouseState &ms = mpMouse->getMouseState();
  ms.width = width;
  ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void Application::windowClosed(Ogre::RenderWindow* rw)
{
  //Only close for window that created OIS (the main window in these demos)
  if( rw == mpWindow )
  {
    if( mpInputManager )
    {
      mpInputManager->destroyInputObject( mpMouse );
      mpInputManager->destroyInputObject( mpKeyboard );

      OIS::InputManager::destroyInputSystem(mpInputManager);
      mpInputManager = 0;
    }
  }
}

bool Application::handleKeyboardInput(const Ogre::FrameEvent& evt)
{
  // Quit if Q or ESC are pressed
  if( mpKeyboard->isKeyDown(OIS::KC_Q) || mpKeyboard->isKeyDown(OIS::KC_ESCAPE) )
    return false;

  // Show stats if F is pressed
  if( mpKeyboard->isKeyDown(OIS::KC_F) && mTimeUntilNextToggle <= 0 )
  {
    mStatsOn = !mStatsOn;
    showDebugOverlay(mStatsOn);
    mTimeUntilNextToggle = 1;
  }

  // Toggle between SOLID and WIREFRAME when W is pressed
  if(mpKeyboard->isKeyDown(OIS::KC_W) && mTimeUntilNextToggle <=0)
  {
    mSceneDetailIndex = (mSceneDetailIndex+1)%2 ;
    switch(mSceneDetailIndex) {
        case 0 : mpCamera->setPolygonMode(Ogre::PM_SOLID); break;
        case 1 : mpCamera->setPolygonMode(Ogre::PM_WIREFRAME); break;
    }
    mTimeUntilNextToggle = 0.5;
  }

  // Toggle camera position/orientation when P is pressed
  static bool displayCameraDetails = false;
  if(mpKeyboard->isKeyDown(OIS::KC_P) && mTimeUntilNextToggle <= 0)
  {
    displayCameraDetails = !displayCameraDetails;
    mTimeUntilNextToggle = 0.5;
    if (!displayCameraDetails)
      mDebugText = "";
    else
      mDebugText = "P: " + Ogre::StringConverter::toString(mpCamera->getDerivedPosition()) +
      " " + "O: " + Ogre::StringConverter::toString(mpCamera->getDerivedOrientation());
  }

  // Return true to continue rendering
  return true;
}

bool Application::handleMouseInput(const Ogre::FrameEvent& evt)
{
  const OIS::MouseState &ms = mpMouse->getMouseState();
  int wheel = ms.Z.rel;

  // :TODO: Handle mouse input for main menu

  return true;
}



// Override frameStarted event to process that (don't care about frameEnded)
bool Application::frameStarted(const Ogre::FrameEvent& evt)
{
  if(mpWindow->isClosed())	return false;

  //Need to capture/update each device
  mpKeyboard->capture();
  mpMouse->capture();

  // one of the input modes is immediate, so setup what is needed for immediate movement
  if (mTimeUntilNextToggle >= 0) mTimeUntilNextToggle -= evt.timeSinceLastFrame;

  //Check to see which device is not buffered, and handle it
  if (handleKeyboardInput(evt) == false)
    return false;
  if (handleMouseInput(evt) == false)
    return false;

  // Update the current GameState
  if (mpCurrentState) 
    mpCurrentState->update(evt.timeSinceLastFrame);

  return true;
}

bool Application::frameEnded(const Ogre::FrameEvent& evt)
{
  updateStats();
  return true;
}

void Application::createScene(void)
{
  // Create our custom singletons
  new TowerFactory();

  // Ambient Light
  mpSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

  // Sun
  Ogre::Light* light = mpSceneMgr->createLight("Sun");
  light->setType(Ogre::Light::LT_DIRECTIONAL);
  light->setDirection(0, -1, -0.5f);
  light->setDiffuseColour(1, 1, 1);

  // Setup all the static members of Level (and its dependencies)
  Level::staticSetup(mpSceneMgr);

  //int top, left, width, height;
  //mpCamera->getViewport()->getActualDimensions(top, left, width, height);

  //int y = height/2 - 80;
  //int x = width/2 - 50;

  //// Main Menu
  //mpMenu = new GUI::Window("MainMenu");
  //
  //// New Button
  //GUI::Label* lblNew = new GUI::Label("New", mpMenu);
  //lblNew->setPosition(x, y);
  //y += 40;

  //// Level Editor Button
  //GUI::Label* lblEdit = new GUI::Label("Editor", mpMenu);
  //lblEdit->setPosition(x, y);
  //y += 40;

  //// Exit Button
  //GUI::Label* lblExit = new GUI::Label("Exit", mpMenu);
  //lblExit->setPosition(x, y);

  //mpMenu->show();
  
  Level* level = new Level;
  level->initialize(mpCamera, mpMouse, mpKeyboard);
  level->load("Test");

  mpCurrentState = level;
  //mpLevel->save();
}