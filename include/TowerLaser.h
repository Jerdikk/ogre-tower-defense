#pragma once
#include "Tower.h"

class TowerBase;

class TowerLaser : public Tower
{
public:
  TowerLaser(TowerBase* parent);
  ~TowerLaser(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::ManualObject* mpShotGraphics;
};
