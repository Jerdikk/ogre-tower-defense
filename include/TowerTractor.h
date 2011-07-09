#pragma once
#include "Tower.h"

class TowerBase;

class TowerTractor : public Tower
{
public:
  TowerTractor(TowerBase* parent);
  ~TowerTractor(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::ManualObject* mpShotGraphics;
  Ogre::SceneNode* mpShotNode;
  unsigned int mTargetId;
};
