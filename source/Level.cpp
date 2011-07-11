#include "Level.h"
#include <Ogre.h>
#include "TowerFactory.h"
#include "NodeGraphic.h"
#include "NodeLineGraphic.h"
#include "Label.h"
#include "Common.h"
#include "AlienFactory.h"
#include "ThreadManager.h"
#include "gui.h"

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

Ogre::SceneManager* Level::mpsSceneMgr = NULL;
Ogre::RaySceneQuery* Level::mpsRsq = NULL;

/***********************************************************************
 *                             PUBLIC                                  *
 ***********************************************************************/
Level::Level(void)
{
  if (AlienFactory::getSingletonPtr() == NULL) new AlienFactory();

  mpMutex = new Mutex();
  mNewDefaultPaths = 0;
  mNewAlienPaths = 0;
  mWaitingOnNewPaths = false;
  mThreadId = 0;
  mName = "";
  mResources = 10000;
  mLeftButtonDown = false;
  mRightButtonDown = false;
  mMoveScale = 0.0f;
  mRotScale = 0.0f;
  mTimeUntilNextToggle = 0;
  mMoveSpeed = 250;
  mRotateSpeed = 36;
  mLeftButtonDown = false;
  mRightButtonDown = false;
  mPaused = false;

  mpCamera = NULL;
  mpSelectedTower = NULL;
  mpLevelGui = NULL;
  mpPurchaseGui = NULL;
  mpUpgradeGui = NULL;
  lblResources = NULL;
  lblDebug = NULL;
  mpCursorRed = NULL;
  mpCursorGreen = NULL;
}

Level::~Level(void)
{
  // Destroy Ground
  mpsSceneMgr->destroyManualObject("Ground");

  // Destroy Grid
  mpsSceneMgr->destroyManualObject("Grid");

  // Destroy all TowerBases
  for (size_t i = 0; i < mTowerBases.size(); ++i)
    delete mTowerBases[i];
  mTowerBases.clear();

  // Destroy all Other Graphics
  for (size_t i = 0; i < mOtherGraphics.size(); ++i)
  {
    Ogre::ManualObject* mo = mOtherGraphics[i];
    Ogre::SceneNode* node = mo->getParentSceneNode();
    node->detachObject(mo);
    mpsSceneMgr->destroyManualObject(mo);
  }
  mOtherGraphics.clear();
}

void Level::initialize(
  Ogre::Camera* camera,
  OIS::Mouse* mouse,	
  OIS::Keyboard* keyboard)
{
  mpCamera = camera;
  mpMouse = mouse;
  mpKeyboard = keyboard;

  // Get the OverlayElements for the cursor red/green
  mpCursorRed = NULL;
  mpCursorGreen = NULL;
  Ogre::Overlay* overlay = Ogre::OverlayManager::getSingleton().getByName("Overlay/Cursor");
  if (overlay)
  {
    mpCursorRed = overlay->getChild("Overlay/Cursor/CursorRed");
    mpCursorGreen = overlay->getChild("Overlay/Cursor/CursorGreen");

    overlay->show();
    mpCursorRed->show();
    mpCursorGreen->hide();
  }

  // Initialize the GUI
  _initGui();
}

void Level::staticSetup(Ogre::SceneManager* sceneMgr)
{
  if (mpsRsq) mpsSceneMgr->destroyQuery(mpsRsq);

  mpsSceneMgr = sceneMgr;
  mpsRsq = mpsSceneMgr->createRayQuery(Ogre::Ray());

  Tower::initialize(mpsSceneMgr);
  TowerBase::initialize(mpsSceneMgr);
  NodeGraphic::initialize(mpsSceneMgr);
  NodeLineGraphic::initialize(mpsSceneMgr);
  Alien::initialize(mpsSceneMgr);
}

bool Level::load(const Ogre::String& name)
{
  bool success = false;

  if (mpsSceneMgr != NULL && _parseFile(name))
  {
    mName = name;
    
    // Create Grid
    Ogre::ManualObject* grid = mpsSceneMgr->createManualObject("Grid");
    grid->begin("GridMaterial", Ogre::RenderOperation::OT_LINE_LIST);

    int width = mLevelData.width();
    int height = mLevelData.height();

    Ogre::Real worldWidth = width * CELL_WIDTH;
    Ogre::Real worldHeight = -height * CELL_WIDTH;

    // Draw the height lines
    for (int i = 0; i <= width; i++)
    {
      grid->position(cornerPosFromCoords(0, i));
      grid->position(cornerPosFromCoords(height, i));
    }

    // Draw the width lines 
    for (int i = 0; i <= height; i++)
    {
      grid->position(cornerPosFromCoords(i, 0));
      grid->position(cornerPosFromCoords(i, width));
    }
    grid->end();
    grid->setQueryFlags(MASK_CURSOR_TARGET);
    mExtents = Ogre::AxisAlignedBox(-500, -1, worldHeight+400, worldWidth-200, 0, 600);
    grid->setBoundingBox(mExtents);
    mpsSceneMgr->getRootSceneNode()->attachObject(grid);

    // Create "ground" for cursor to project onto
    Ogre::ManualObject* ground = mpsSceneMgr->createManualObject("Ground");
    ground->begin("Material/Ground", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    ground->position(cornerPosFromCoords(0, 0)          + Ogre::Vector3::NEGATIVE_UNIT_Y);
    ground->position(cornerPosFromCoords(0, width)      + Ogre::Vector3::NEGATIVE_UNIT_Y);
    ground->position(cornerPosFromCoords(height, width) + Ogre::Vector3::NEGATIVE_UNIT_Y);
    ground->position(cornerPosFromCoords(height, 0)     + Ogre::Vector3::NEGATIVE_UNIT_Y);
    ground->quad(0, 1, 2, 3);
    ground->end();
    ground->setQueryFlags(MASK_CURSOR_TARGET);
    ground->setBoundingBox(mExtents);
    mpsSceneMgr->getRootSceneNode()->attachObject(ground);

    // Level Data
    for (int row = 0; row < mLevelData.height(); ++row)
    {
      for (int col = 0; col < mLevelData.width(); ++col)
      {
        assert(mLevelData.isValid(row, col));
        
        int val = mLevelData[row][col];
        
        // Tower Base
        if (val == TOWER_BASE || val == TOWER_BASE_OPEN)
        {
          bool passable = (val == TOWER_BASE_OPEN);
          mTowerBases.push_back(new TowerBase(row, col, passable));
        }
        // Paths
        else if (val == OPEN)
        {
          // TODO - Create better graphics for paths
          Ogre::ManualObject* path = mpsSceneMgr->createManualObject("Path_" + Ogre::StringConverter::toString(col) +
                                                                     "_" + Ogre::StringConverter::toString(row));
          Ogre::Vector3 min = cornerPosFromCoords(row+1, col+0);
          Ogre::Vector3 max = cornerPosFromCoords(row+0, col+1);
          path->begin("Material/OpenNode", Ogre::RenderOperation::OT_TRIANGLE_LIST);
          path->position(min);
          path->position(min.x, 0, max.z);
          path->position(max);
          path->position(max.x, 0, min.z);
          path->quad(0, 1, 2, 3);
          path->end();
          path->setQueryFlags(MASK_CURSOR_TARGET);
          path->setBoundingBox(Ogre::AxisAlignedBox(min.x, -1, min.z, max.x, 0, max.z));
          mpsSceneMgr->getRootSceneNode()->attachObject(path);
          mOtherGraphics.push_back(path);
        }
        // Power Cores
        else if (val == POWER_CORE)
        {
          mPowerCore = GridNode(row, col);

          // TODO - Create better graphics for power core
          Ogre::ManualObject* path = mpsSceneMgr->createManualObject("Path_" + Ogre::StringConverter::toString(col) +
                                                                     "_" + Ogre::StringConverter::toString(row));
          Ogre::Vector3 min = cornerPosFromCoords(row+1, col+0);
          Ogre::Vector3 max = cornerPosFromCoords(row+0, col+1);
          path->begin("Material/PowerCore/Base", Ogre::RenderOperation::OT_TRIANGLE_LIST);
          path->position(min);
          path->position(min.x, 0, max.z);
          path->position(max);
          path->position(max.x, 0, min.z);
          path->quad(0, 1, 2, 3);
          path->end();
          path->setQueryFlags(MASK_CURSOR_TARGET);
          path->setBoundingBox(Ogre::AxisAlignedBox(min.x, -1, min.z, max.x, 0, max.z));
          mpsSceneMgr->getRootSceneNode()->attachObject(path);
          mOtherGraphics.push_back(path);
        }
      }
    }

    // Find the initial path - Call the background thread directly
    ThreadArgs* args = new ThreadArgs();
    args->level = this;
    args->genDefaultPath = true;
    args->avoidTowers = true;
    args->map = mLevelData;
    args->entrance = mEnter;
    args->exit = mExit;
    args->cores = mPowerCore;
    for (AlienItr itr = mAliens.begin(); itr != mAliens.end(); ++itr)
      args->alienPositions[itr->first] = std::make_pair(itr->second->getPosition(), ThreadArgs::CORES);
    backgroundThread(args);
    // backgroundThread() destroys the ThreadArgs object, so it's now invalid

    success = true;
  }

  return success;
}

bool Level::save(void)
{
  bool success = false;

  std::string filename = ".\\media\\" + mName + " - copy.lvl";
  std::fstream fp;
  fp.open(filename.c_str(), std::ios::out | std::ios::binary);
  
  if (fp)
  {
    // Header
    fp << "Width: " << mLevelData.width() << std::endl;
    fp << "Height: " << mLevelData.height() << std::endl;
    fp << "Enter: " << mEnter.row << " " << mEnter.col << std::endl;
    fp << "Exit: " << mExit.row << " " << mExit.col << std::endl;
    fp << std::endl;

    // Data
    for (int y = mLevelData.height()-1; y >= 0; --y)
    {
      for (int x = 0; x < mLevelData.width(); ++x)
      {
        fp << mLevelData.getValue(mLevelData.index(y, x)) << " ";
      }
      fp << std::endl;
    }
    fp << std::endl;

    // Waves
    // TODO: writeWaves not yet implemented
    success = AlienFactory::getSingleton().writeWaves(fp);
  }

  fp.close();
  return success;
}

bool Level::update(float t)
{
  // :TEMP: Prevent excessing frame rates that result from the device
  // losing focus
  if (t > 0.1f) t = 0.1f;

  if (lblResources) 
    lblResources->setText("Resources: " + Ogre::StringConverter::toString(mResources));

  // One of the input modes is immediate, so setup what is needed for immediate movement
  if (mTimeUntilNextToggle >= 0) 
    mTimeUntilNextToggle -= t;

  // Move about 100 units per second
  mMoveScale = mMoveSpeed * t;

  // Take about 10 seconds for full rotation
  mRotScale = mRotateSpeed * t;

  mRotX = 0;
  mRotY = 0;
  mTranslateVector = Ogre::Vector3::ZERO;

  if (!mPaused)
  {
    // Update aliens
    _updateAliens(t);

    // Update towers
    TowerFactory::getSingleton().updateAllTowers(t);
  }

  // Handle user input
  _handleMouseInput(t);
  _handleKeyboardInput(t);

  // Update the camera
  _moveCamera();
  _moveCursor();

  return true;
}

void Level::onClick(GUI::Label* label)
{
}

/***********************************************************************
 *                           PROTECTED                                 *
 ***********************************************************************/
void Level::notifyDefaultPaths(const Path& toData, const Path& toExit)
{
  SCOPELOCK(mpMutex);
  mNewDefaultPathToData = toData;
  mNewDefaultPathToExit = toExit;
  InterlockedExchange(&mNewDefaultPaths, 1);
}

void Level::notifyPath(unsigned int id, const Path& path)
{
  SCOPELOCK(mpMutex);
  mNewPositions[id] = path;
  InterlockedExchange(&mNewAlienPaths, 1);
}

void Level::onLabelClicked(GUI::Widget* label)
{
}

/***********************************************************************
 *                            PRIVATE                                  *
 ***********************************************************************/
bool Level::_parseFile(const Ogre::String& name)
{
  bool success = true;

  std::string filename = ".\\media\\levels\\" + name + ".lvl";
  std::ifstream fp;
  fp.open(filename.c_str(), std::ios::in | std::ios::binary);

  if (fp)
  {
    enum ParseState 
    {
      PROPERTIES,
      DATA,
      WAVE,
    } state;

    Ogre::DataStreamPtr stream(OGRE_NEW Ogre::FileStreamDataStream(filename, &fp, false));
    Ogre::String line;
    Ogre::String substr;
    int width = 0;
    int height = 0;
    bool resized = false;
    state = PROPERTIES;

    // PROPERTIES
    if (state == PROPERTIES)
    {
      while (!fp.eof() && state == PROPERTIES)
      {
        line = stream->getLine();
        
        if (line.length() > 0)
        {
          if (getStrAfterIfMatch(line, "Width:", substr))
          {
            width = Ogre::StringConverter::parseInt(substr);
          }
          else if (getStrAfterIfMatch(line, "Height:", substr))
          {
            height = Ogre::StringConverter::parseInt(substr);
          }
          else if (getStrAfterIfMatch(line, "Enter:", substr)) 
          {
            Ogre::Vector2 enter = Ogre::StringConverter::parseVector2(substr);
            mEnter.row = static_cast<int>(enter.x);
            mEnter.col = static_cast<int>(enter.y);
            AlienFactory::getSingleton().setStartingPoint(mEnter);
          }
          else if (getStrAfterIfMatch(line, "Exit:", substr))
          {
            Ogre::Vector2 exit = Ogre::StringConverter::parseVector2(substr);
            mExit.row = static_cast<int>(exit.x);
            mExit.col = static_cast<int>(exit.y);
          }
        }
        else
        {
          state = DATA;
        }
      }
    } // properties

    // DATA
    if (width && height) 
    {
      mLevelData.resize (width, height);
  
      // Level data.  Sinec we're reading the data from top to bottom, we actually
      // need to start at the last row and work down to 0
      for (int row = height-1; row >= 0; --row)
      {
        // Make sure we're not beyond the end of file
        if (fp.eof())
        {
          success = false;
          break;
        }

        // Get the next line
        line = stream->getLine();

        // Parse the data.  Split the string to get each value, then convert them
        // to ints and add them to the mLevelData Map.
        Ogre::vector<Ogre::String>::type strVals = Ogre::StringUtil::split(line);
        for (size_t i = 0; i < strVals.size(); ++i)
        {
          mLevelData[row][(int)i] = Ogre::StringConverter::parseInt(strVals[i]);
        }
      }

      state = WAVE;
    } // if (width & higt)
    else 
    {
      success = false;
    } // data

    if (state == WAVE)
    {
      // Parse wave data.
      while (success && !fp.eof())
      {
        line = stream->getLine();

        if (line == "Wave")
        {
          success = AlienFactory::getSingleton().parseWave(stream);
        }
      }
    }
    fp.close();

    success = true;
  }
  return success;
}

void Level::_initGui(void)
{
  mpLevelGui = new GUI::Window("LevelGui");
  mpPurchaseGui = new GUI::Window("PurchaseGui");
  mpUpgradeGui = new GUI::Window("UpgradeGui");

  mpPurchaseGui->hide();
  mpUpgradeGui->hide();

  // Debug
  lblDebug = new GUI::Label(mpLevelGui);
  lblDebug->setPosition(30, 30);
  lblDebug->setText("Debug Label");

  // Resources
  lblResources = new GUI::Label("Resources: ", mpLevelGui);
  lblResources->setPosition(30, 10);

  // Towers LabelGroup
  grpTowerTypes = new GUI::LabelGroup();
  GUI::Label* towerLabel;
  int y = 300;

  towerLabel = new GUI::Label("Laser", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_1, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Flame", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_2, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Cannon", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_3, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Barrier", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_4, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Temporal", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_5, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Tractor", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_6, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Energy Bomb", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_7, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Armory", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_8, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Quantum", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_9, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Research", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_0, towerLabel);
  y += 20;

  towerLabel = new GUI::Label("Cancel", mpPurchaseGui);
  towerLabel->setPosition(10, y);
  grpTowerTypes->addLabel(OIS::KC_ESCAPE, towerLabel);
  y += 20;

  // Upgrade LabelGroup
  grpUpgrades = new GUI::LabelGroup();
  GUI::Label* upgradeLabel;
  y = 400;

  upgradeLabel = new GUI::Label("Upgrade", mpUpgradeGui);
  upgradeLabel->setPosition(10, y);
  grpUpgrades->addLabel(OIS::KC_1, upgradeLabel);
  y += 20;

  upgradeLabel = new GUI::Label("Sell", mpUpgradeGui);
  upgradeLabel->setPosition(10, y);
  grpUpgrades->addLabel(OIS::KC_2, upgradeLabel);
  y += 20;

  upgradeLabel = new GUI::Label("Cancel", mpUpgradeGui);
  upgradeLabel->setPosition(10, y);
  grpUpgrades->addLabel(OIS::KC_3, upgradeLabel);
  y += 20;
}

void Level::_updateAliens(float t)
{
   // Notify the AlienFactory to handle the next wave
  Aliens newAliens;
  AlienFactory::getSingleton().handleNextWave(t, newAliens);
  if (!newAliens.empty())
  {
    mAliens.insert(newAliens.begin(), newAliens.end());
  }

  // Check for new default paths from background thread
  if (InterlockedCompareExchange(&mNewDefaultPaths, 0, 1) == 1)
  {
    SCOPELOCK(mpMutex);
    mDefaultPathToData = mNewDefaultPathToData;
    mDefaultPathToExit = mNewDefaultPathToExit;
    mWaitingOnNewPaths = false;
  }

  // Check for new alien paths from background thread
  std::map<unsigned int, Path> newPositions;
  if (InterlockedCompareExchange(&mNewAlienPaths, 0, 1) == 1)
  {
    SCOPELOCK(mpMutex);
    newPositions.insert(mNewPositions.begin(), mNewPositions.end());
    mNewPositions.clear();
  }

  // Continue processing the new positions until there are no more.
  while (!newPositions.empty())
  {
    unsigned int id = newPositions.begin()->first;
    Path path = newPositions.begin()->second;
    newPositions.erase(newPositions.begin());

    // Update this Alien's path
    AlienItr itr = mAliens.find(id);
    if (itr != mAliens.end())
    {
      itr->second->setPath(path);
    }
  }

  // Update Aliens
  bool itrAdvanced = false;
  AlienItr itr = mAliens.begin();
  while (itr != mAliens.end())
  {
    itrAdvanced = false;
    Alien* alien = itr->second;
    
    // Update the alien
    alien->update(t);

    // Check the alien's state
    Alien::State state = alien->getState();

    // If the alien has entered the level, give it the default path to the data
    if (state == Alien::AWAITING_DATA_PATH && !mWaitingOnNewPaths)
    {
      alien->setPath(mDefaultPathToData);
    }

    // If the alien reach the data, give it the default path to the exit
    else if (state == Alien::AWAITING_EXIT_PATH && !mWaitingOnNewPaths)
    {
      alien->setPath(mDefaultPathToExit);
    }

    // The alien has exited the level or died.
    else if (state == Alien::EXITED || state == Alien::DEAD)
    {
      AlienFactory::getSingleton().destroyAlien(alien);
      itr = mAliens.erase(itr);
      itrAdvanced = true;

      if (state == Alien::EXITED)
      {
        // :TODO: Penalize player for alien escaping
      }
    }

    // Advance the iterator if it wasn't advanced already.
    if (!itrAdvanced)
      ++itr;
  }
}

void Level::_handleMouseInput(float t)
{
  if (mpPurchaseGui->isVisible() || mpUpgradeGui->isVisible())
  {
    // If the GUI is already visible, handle GUI input
    _handleGui(t);
    return;
  }

  // Rotation factors, may not be used if the second mouse button is pressed
  // 2nd mouse button - slide, otherwise rotate
  const OIS::MouseState &ms = mpMouse->getMouseState();

  // Determine if either mouse button was clicked.  We're looking to see if the
  // button was down last frame and not this frame.  That's a click.
  bool leftClick = false;
  bool rightClick = false;
  if (mLeftButtonDown && !ms.buttonDown(OIS::MB_Left)) leftClick = true;
  if (mRightButtonDown && !ms.buttonDown(OIS::MB_Right)) rightClick = true;

  // Update the status of each mouse button for this frame
  mLeftButtonDown = ms.buttonDown(OIS::MB_Left);
  mRightButtonDown = ms.buttonDown(OIS::MB_Right);

  if (leftClick)
  {    
    // Update the Level with the current mouse state so that it can take any actions it needs to.
    Ogre::Ray ray;
    ray.setOrigin(mpCamera->getDerivedPosition());
    ray.setDirection(mpCamera->getDerivedDirection());

    // Execute the mouse query to see what was clicked on
    mpsRsq->setQueryMask(MASK_TOWER_BASE);
    mpsRsq->setSortByDistance(true);
    mpsRsq->setRay(ray);
    Ogre::RaySceneQueryResult results = mpsRsq->execute();
    Ogre::RaySceneQueryResult::iterator itr = results.begin();
    
    bool mapChanged = false;
    bool handled = false;
    while (itr != results.end() && !handled)
    {
      Ogre::UserObjectBindings& object = itr->movable->getUserObjectBindings();
      if (!object.getUserAny().isEmpty() && object.getUserAny().getType() == typeid(TowerBase*))
      {
        handled = true;
        TowerBase* base = Ogre::any_cast<TowerBase*>(object.getUserAny());
        mpSelectedTower = base;

        // Show the appropriate GUI if either mouse button was clicked
        _showGui();

      } // if (TowerBase)

      ++itr;
    } // while (results) 
  } // if (left or right click)

  else
  {
    // Mouse Wheel
    int wheel = ms.Z.rel;

    mTranslateVector.x += ms.X.rel * mMoveScale;
    mTranslateVector.z += ms.Y.rel * mMoveScale;
  }
}

void Level::_handleKeyboardInput(float t)
{
  static Ogre::Real sTimeSinceLastToggle = 0;
  sTimeSinceLastToggle += t;

  //if(keyboard->isKeyDown(OIS::KC_LEFT))
  //  mTranslateVector.x = -mMoveScale;	// Move camera left

  //if(keyboard->isKeyDown(OIS::KC_RIGHT))
  //  mTranslateVector.x = mMoveScale;	// Move camera RIGHT

  //if(keyboard->isKeyDown(OIS::KC_UP))
  //  mTranslateVector.z = -mMoveScale;	// Move camera forward

  //if(keyboard->isKeyDown(OIS::KC_DOWN))
  //  mTranslateVector.z = mMoveScale;	// Move camera backward

  if(mpKeyboard->isKeyDown(OIS::KC_PGUP))
    mTranslateVector.y = mMoveScale;	// Move camera up

  if(mpKeyboard->isKeyDown(OIS::KC_PGDOWN))
    mTranslateVector.y = -mMoveScale;	// Move camera down

  if(mpKeyboard->isKeyDown(OIS::KC_RIGHT))
    mRotX = -mRotScale;

  if(mpKeyboard->isKeyDown(OIS::KC_LEFT))
    mRotX = mRotScale;

  if (sTimeSinceLastToggle >= 0.1)
  {
    sTimeSinceLastToggle = 0;

    if(mpKeyboard->isKeyDown(OIS::KC_P))
      mPaused = !mPaused;
  }
}

void Level::_moveCamera()
{
  Ogre::SceneNode* camNode = mpCamera->getParentSceneNode();

  // Make all the changes to the camera
  // Note that YAW direction is around a fixed axis (freelook style) rather than a natural YAW
  //(e.g. airplane)
  camNode->yaw(mRotX);
  camNode->pitch(mRotY);
  camNode->translate(mTranslateVector, Ogre::Node::TS_LOCAL);

  Ogre::Vector3 camPos = camNode->getPosition();

  // See if the camera needs to be moved to keep it within the extents of the level
  bool adjustCam = false;
  if (camPos.x < mExtents.getMinimum().x) {camPos.x = mExtents.getMinimum().x; adjustCam = true;}
  if (camPos.x > mExtents.getMaximum().x) {camPos.x = mExtents.getMaximum().x; adjustCam = true;}
  if (camPos.z < mExtents.getMinimum().z) {camPos.z = mExtents.getMinimum().z; adjustCam = true;}
  if (camPos.z > mExtents.getMaximum().z) {camPos.z = mExtents.getMaximum().z; adjustCam = true;}

  // Move the camera if necessary
  if (adjustCam) camNode->setPosition(camPos);
}

void Level::_moveCursor()
{
  if (mpCursorGreen && mpCursorRed)
  {
    // Create a static Plane representing the ground
    static Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);

    Ogre::SceneNode* camNode = mpCamera->getParentSceneNode();

    // Determine which cursor image to use
    Ogre::String imageName = "crosshair_red.png";
    Ogre::Ray ray;
    ray.setOrigin(camNode->_getDerivedPosition());
    ray.setDirection(mpCamera->getDerivedDirection());

    std::pair<bool, Ogre::Real> intersection = ray.intersects(plane);
    if (intersection.first)
    {
      Ogre::Vector3 point = ray.getPoint(intersection.second);
      if (lblDebug) lblDebug->setText(Ogre::StringConverter::toString(point));
    }

    // Create RaySceneQuery
    mpsRsq->setQueryMask(MASK_TOWER_BASE);
    mpsRsq->setRay(ray);
    Ogre::RaySceneQueryResult rayResults = mpsRsq->execute();
    if (!rayResults.empty()) 
    {
      mpCursorGreen->show();
      mpCursorRed->hide();
    }
    else
    {
      mpCursorGreen->hide();
      mpCursorRed->show();
    }
  }
}

void Level::_projectCursorOnMaterial(const Ogre::String& matName)
{
  //// Only do this if this material is not already a cursor target
  //if (mCursorTargets.find(matName) == mCursorTargets.end())
  //{
  //  Ogre::MaterialPtr material;
  //  Ogre::MaterialManager* MM = Ogre::MaterialManager::getSingletonPtr();

  //  // Either get or load the material
  //  try
  //  {
  //    if (MM->resourceExists(matName)) 
  //      material = static_cast<Ogre::MaterialPtr>(MM->getByName(matName));
  //    else
  //      material = static_cast<Ogre::MaterialPtr>(MM->load(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));    
  //  } 
  //  catch (Ogre::Exception&)
  //  {
  //    std::cout << "Error adding " << matName << " as a target for cursor projection." << std::endl;
  //    return;
  //  }
  //  
  //  //// Add the decal pass to the material
  //  //if (material.isNull() == false)
  //  //{
  //  //  Ogre::Pass* pass = material->getTechnique(0)->createPass();
  //  //  Ogre::TextureUnitState* tsu = pass->createTextureUnitState(mCursorImage);
  //  //  pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  //  //  pass->setDepthBias(2);
  //  //  pass->setLightingEnabled(false);
  //  //  tsu->setProjectiveTexturing(true, mCursorFrustum);
  //  //  tsu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
  //  //  tsu->setTextureBorderColour(Ogre::ColourValue(0, 0, 0, 0));
  //  //  tsu->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_LINEAR, Ogre::FO_NONE);
  //  //  mCursorTargets[matName] = pass;
  //  //}
  //}
}

void Level::_removeCursorFromMaterial(const Ogre::String& matName)
{
  //// Find this material's decal pass in the cursor targets map
  //PassMapItr itr = mCursorTargets.find(matName);
  //if (itr != mCursorTargets.end())
  //{
  //  // This target is no longer being used.  Remove the decal pass from the material, and remove
  //  // the target from the map
  //  itr->second->getParent()->removePass(itr->second->getIndex());
  //  itr = mCursorTargets.erase(itr);
  //}
}

void Level::_handleGui(float t)
{
  bool mapChanged = false;
  const OIS::MouseState &ms = mpMouse->getMouseState();

  // Mouse Wheel
  int wheel = ms.Z.rel;

  // Determine if either mouse button was clicked.  We're looking to see if the
  // button was down last frame and not this frame.  That's a click.
  bool leftClick = false;
  bool rightClick = false;
  if (mLeftButtonDown && !ms.buttonDown(OIS::MB_Left)) leftClick = true;
  if (mRightButtonDown && !ms.buttonDown(OIS::MB_Right)) rightClick = true;

  // Update the status of each mouse button for this frame
  mLeftButtonDown = ms.buttonDown(OIS::MB_Left);
  mRightButtonDown = ms.buttonDown(OIS::MB_Right);

  // If the mouse wheel was not moved and the left button not clicked, do nothing
  if (wheel == 0 && !leftClick && !rightClick)
    return;

  if (mpPurchaseGui->isVisible())
  {
    // Handle Wheel
    if      (wheel > 0) grpTowerTypes->cycleUp();
    else if (wheel < 0) grpTowerTypes->cycleDown();

    // Handle Left Click
    if (leftClick)
    {
      // Hide the GUI.  We're done with it for now.
      mpPurchaseGui->hide();

      // Get the text of the label selected
      Ogre::String text = grpTowerTypes->getCurrentLabel()->getText();

      // If Cancel, just return
      if (text == "Cancel") return;

      // Purchase the selected tower
      int cost = TowerFactory::getSingleton().getPurchaseCost(text);
      mResources -= cost;
      mpSelectedTower->createTower(text);

      // If the tower base was passable, the map has changed
      if (mpSelectedTower->isPassable())
      {
        mapChanged = true;
        mLevelData[mpSelectedTower->getRow()][mpSelectedTower->getCol()] = TOWER_BASE_CLOSED;
      }

      // Deselect this towe base
      mpSelectedTower = NULL;
     
    } // if (leftClick)

    else if (rightClick)
    {
      // If righ click, just hide the GUI as if cancel was pressed
      mpPurchaseGui->hide();
    } // if (rightClick)
  }
  else if (mpUpgradeGui->isVisible())
  {
    // Handle Wheel
    if      (wheel > 0) grpUpgrades->cycleUp();
    else if (wheel < 0) grpUpgrades->cycleDown();

    // Handle Left Click
    if (leftClick)
    {
      // Hide the GUI.  We're done with it for now.
      mpUpgradeGui->hide();

      // Get the text of the label selected
      Ogre::String text = grpUpgrades->getCurrentLabel()->getText();

      // If Cancel, just return
      if (text == "Cancel") return;

      if (text == "Sell")
      {
        // Sell the selected tower
        int cost = TowerFactory::getSingleton().getSellValue(mpSelectedTower->getTowerType(), mpSelectedTower->getTowerLevel());
        mResources += cost;
        mpSelectedTower->sellTower();

        // If the tower base was passable, the map has changed
        if (mpSelectedTower->isPassable())
        {
          mapChanged = true;
          mLevelData[mpSelectedTower->getRow()][mpSelectedTower->getCol()] = TOWER_BASE_OPEN;
        } // if (base isPassable)
      }
      else if (text == "Upgrade")
      {
        // Upgrade the selected tower
        int cost = TowerFactory::getSingleton().getUpgradeCost(mpSelectedTower->getTowerType(), mpSelectedTower->getTowerLevel());
        mResources -= cost;
        mpSelectedTower->upgradeTower();
      }
    } // if (leftClick)

    else if (rightClick)
    {
      // If righ click, just hide the GUI as if cancel was pressed
      mpUpgradeGui->hide();
    } // if (rightClick)
  }

  // If the map changed, kick off the background thread to generate new
  // paths for everything.
  if (mapChanged) 
  {
    SCOPELOCK(mpMutex);
    ThreadArgs* args = new ThreadArgs();
    args->level = this;
    args->genDefaultPath = true;
    args->avoidTowers = true;
    args->map = mLevelData;
    args->entrance = mEnter;
    args->exit = mExit;
    args->cores = mPowerCore;
    for (AlienItr itr = mAliens.begin(); itr != mAliens.end(); ++itr)
    {
      ThreadArgs::DESTINATION dest = ThreadArgs::NONE;
      if (itr->second->getState() == Alien::TRACKING_EXIT)      dest = ThreadArgs::EXIT;
      else if (itr->second->getState() == Alien::TRACKING_DATA) dest = ThreadArgs::CORES;      

      if (dest != ThreadArgs::NONE)
        args->alienPositions[itr->first] = std::make_pair(itr->second->getPosition(), dest);
    }
    std::cout << "Request new paths for " << args->alienPositions.size() << " aliens" << std::endl;
    mThreadId = ThreadManager::instance()->startThread(backgroundThread, args);
    mWaitingOnNewPaths = 0;
  } // if (mapChanged)
}

void Level::_showGui()
{
  if (mpSelectedTower->getTowerType() == "")
  {
    // Empty base.  Show the Purchase GUI
    // :TODO: Populate the GUI with the appropriate options
    mpPurchaseGui->show();
  }
  else
  {
    // This base has a tower.  Show the Upgrade GUI
    // :TODO: Populate the GUI with the appropriate options
    if (TowerFactory::getSingleton().getUpgradeCost(mpSelectedTower->getTowerType(), mpSelectedTower->getTowerLevel()) < 0) 
    {
      // Disable the option to upgrade
      grpUpgrades->getLabel(0)->setEnabled(false);
    }
    else
    {
      // Enable the option to upgrade
      grpUpgrades->getLabel(0)->setEnabled(true);
    }

    mpUpgradeGui->show();
  }
}