#pragma once
#include "Tower.h"

class TowerBase;

class TowerCannon : public Tower
{
public:
  TowerCannon(TowerBase* parent);
  ~TowerCannon(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::ManualObject* mpShotGraphics;
};
