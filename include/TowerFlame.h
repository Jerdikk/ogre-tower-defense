#pragma once
#include "Tower.h"

class TowerBase;

class TowerFlame : public Tower
{
public:
  TowerFlame(TowerBase* parent);
  ~TowerFlame(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::ManualObject* mpShotGraphics;
  Ogre::SceneNode* mpShotNode;
  Ogre::Degree mAoA;  // Angle of Attack
};
