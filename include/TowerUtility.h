#pragma once
#include "Tower.h"

class TowerBase;

class TowerUtility : public Tower
{
public:
  TowerUtility(TowerBase* parent, const Ogre::String& type);
  ~TowerUtility(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);
};
