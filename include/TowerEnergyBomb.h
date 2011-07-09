#pragma once
#include "Tower.h"

class TowerBase;

class TowerEnergyBomb : public Tower
{
public:
  TowerEnergyBomb(TowerBase* parent);
  ~TowerEnergyBomb(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);

protected:
  Ogre::Vector3 getShotTarget(const Ogre::Vector3& targetPos, const Ogre::Vector3& targetVel, bool& valid);

protected:
  const float mShotSpeed;
  Ogre::Entity* mpShotGraphics;
  Ogre::SceneNode* mpShotNode;
  Ogre::Vector3 mShotTarget;
  Ogre::Real mSquaredTargetTolerance;
  bool mShooting;
};
