#pragma once
#include <queue>
#include <OgrePrerequisites.h>
#include <OgreVector2.h>
#include <OgreAxisAlignedBox.h>
#include "Map.h"
#include "gui_predef.h"

namespace OIS {class Mouse;}
namespace OIS {class Keyboard;}
class TowerBase;
class NodeGraphic;
class NodeLineGraphic;
class Alien;
class Mutex;
typedef std::vector<TowerBase*> TowerBaseList;
typedef std::vector<NodeGraphic*> NodeGraphicList;

class Level
{
public:
  Level(Ogre::Camera* camera);
  ~Level(void);

  static void initialize(Ogre::SceneManager* sceneMgr);

  // Load a new area from file
  bool load(const Ogre::String& name);

  // Save the level to file
  bool save(void);

  // Update the level each frame.
  // @param t Time since last frame
  // @param mouse Pointer to the mouse object
  // @param keyboard Pointer to the keyboard object
  void update(float t, OIS::Mouse* mouse,	OIS::Keyboard* keyboard);

  // INHERITED FROM LABELCALLBACK
  void onClick(GUI::Label* label);

protected:
  struct ThreadArgs
  {
    Level* level;
    bool genDefaultPath;
    bool avoidTowers;
    Map map;
    GridNode entrance;
    GridNode exit;
    GridNode cores;
    enum DESTINATION {NONE, CORES, EXIT};
    std::map<unsigned int, std::pair<Ogre::Vector3, DESTINATION> > alienPositions;
  };

  /** THE FOLLOWING FUNCTIONS ARE USED TO PASS DATA BACK AND FORTH TO A BACKGROUND THREAD
      WHICH WILL DO THE PATH PLANNING. **/

  // Will run in a background thread to do path planning
  friend void backgroundThread(void*);

  // Callback for the threadFunc when a new default path is computed
  void notifyDefaultPaths(const Path& toData, const Path& toExit);

  // Callback for the threaFunc when a new path is computed for an alien
  void notifyPath(unsigned int id, const Path& path);

  // Callback for when a Label is clicked
  void onLabelClicked(GUI::Widget* label);

private:
  Level(void) {}
  Level(const Level& ) {}
  Level& operator = (const Level&) {}

  // Private initialization methods
  bool _parseFile(const Ogre::String& name);
  void _initGui(void);

  // Private update methods
  void _updateAliens(float t);
  void _handleMouseInput(float t, OIS::Mouse* mouse);
  void _handleKeyboardInput(float t, OIS::Keyboard* keyboard);
  void _moveCamera();
  void _moveCursor();
  void _projectCursorOnMaterial(const Ogre::String& matName);
  void _removeCursorFromMaterial(const Ogre::String& matName);
  void _handleGui(float t, OIS::Mouse* mouse);

  void _showGui();

private:
  static Ogre::SceneManager* mpsSceneMgr;
  static Ogre::RaySceneQuery* mpsRsq;

  // Thread Related
  Mutex* mpMutex;
  unsigned long mThreadId;
  std::map<unsigned int, Path> mNewPositions;
  long mNewDefaultPaths;
  long mNewAlienPaths;
  bool mWaitingOnNewPaths;
  Path mNewDefaultPathToData;
  Path mNewDefaultPathToExit;

  // Cursor Related
  Ogre::OverlayElement* mpCursorRed;
  Ogre::OverlayElement* mpCursorGreen;

  // Input Related
  Ogre::Camera*   mpCamera;     // Owned by Application
  Ogre::Radian    mRotX;
  Ogre::Radian    mRotY;
  Ogre::Real      mMoveScale;
  Ogre::Real      mMoveSpeed;
	Ogre::Degree    mRotateSpeed;
  Ogre::Vector3   mTranslateVector;
  Ogre::Degree    mRotScale;
  Ogre::Real      mTimeUntilNextToggle;
  bool            mLeftButtonDown;
  bool            mRightButtonDown;
  bool            mPaused;

  // Level Characteristics
  int mResources;
  Ogre::String mName;
  GridNode mEnter;
  GridNode mExit;
  GridNode mPowerCore;
  Ogre::AxisAlignedBox mExtents;
  Map mLevelData;
  Path mDefaultPathToData;
  Path mDefaultPathToExit;

  // Towers
  TowerBaseList mTowerBases;
  std::vector<Ogre::ManualObject*> mOtherGraphics;
  TowerBase* mpSelectedTower;

  // Aliens
  Aliens mAliens;

  // Gui
  GUI::Label* lblDebug;
  GUI::Window* mpLevelGui;
  GUI::Window* mpPurchaseGui;
  GUI::Window* mpUpgradeGui;
  GUI::Label* lblResources;
  GUI::LabelGroup* grpTowerTypes;
  GUI::LabelGroup* grpUpgrades;
};