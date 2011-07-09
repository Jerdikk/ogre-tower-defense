#pragma once
#include <map>
#include <OgrePrerequisites.h>
#include "Common.h"

class Alien;

class StatusBar
{
public:
  StatusBar(Alien* alien);
  ~StatusBar(void);

  // Statics
  static void initialize(Ogre::SceneManager* sceneMgr);

  // Update the StatusBar's position so that it follows the Alien,
  // and update the displayed health ration (from 0 to 1)
  void update(const Ogre::Vector3& position, float healthRatio);

protected:
  static std::map<Ogre::String, Ogre::BillboardSet*> msBBSets;
  Ogre::BillboardSet* mpParentSet;
  Ogre::Billboard* mpBar;
};