#pragma once
#include "Tower.h"

class TowerBase;

class TowerTemporal : public Tower
{
public:
  TowerTemporal(TowerBase* parent);
  ~TowerTemporal(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::ManualObject* mpShotGraphics;
  std::vector<Ogre::Vector3> mRingVertices;
};
