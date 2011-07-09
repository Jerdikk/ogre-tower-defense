#pragma once
#include <OgrePrerequisites.h>
#include <OgreUserObjectBindings.h>
#include "Tower.h"

class TowerBase : public Ogre::UserObjectBindings
{
public:
  // Constructor
  // row:  The row index representing this tower base in the map
  // col:  The column index representing this tower base in the map
  // passable:  If true, this tower base will be treated as one that
  //       aliens can pass over when no tower is built on it.
  TowerBase(int row, int col, bool passable = false);

  // Destructor
  ~TowerBase(void);

  static void initialize(Ogre::SceneManager* sceneMgr);
  virtual const Ogre::String& getTypeName(void) const;  // Inherited

  // Returns the type of tower on this base.
  const Ogre::String& getTowerType(void) const;

  // Returns the level of the tower on this base.
  int getTowerLevel(void) const;

  // Return the SceneNode for this base
  Ogre::SceneNode* getNode(void) const {return mpNode;}

  // Add a new tower to this base.  This will only proceed if the base
  // currently has no tower on it.
  void createTower(const Ogre::String& type);

  // Upgrade the current tower to the next level.  This will only work 
  // if the base currently has a tower on it.
  void upgradeTower(void);

  // Sell the current tower.  This will only work if the base currently 
  // has a tower on it.
  void sellTower(void);

  // Returns true if this is a passable tower base.  Passable tower bases
  // are bases that aliens can pass over when there is no tower on it.
  // NOTE - This method returning true does NOT mean that the base does not
  // have a tower on it, just that it is a base capable of being passed over.
  bool isPassable(void) const {return mPassable;}

  // Get pointer to the Tower on this base.  This returns a const pointer
  // so that it can't be modified.
  const Tower* getTower(void) const {return mpTower;}

  // Return the row index for this tower base
  int getRow(void) const {return mRow;}

  // Return the column index for this tower base
  int getCol(void) const {return mCol;}

private:
  bool createGraphics();
  void updateMaterial();

private:
  static Ogre::SceneManager* mpsSceneMgr;
  static int msCounter;

  bool mPassable;
  int mRow;
  int mCol;
  Ogre::ManualObject* mpGraphics;
  Ogre::SceneNode* mpNode;
  Tower* mpTower;
};
