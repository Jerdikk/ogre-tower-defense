#pragma once
#include <OgreSingleton.h>
#include "Tower.h"
#include "TowerBase.h"
#include "common.h"

class TowerFactory : public Ogre::Singleton<TowerFactory>
{
public:
  TowerFactory(void);
  ~TowerFactory(void);

  // Destroy a specific tower
  void destroyTower(Tower* tower);

  // Create a tower of the specified type.  This tower will be owned by the TowerFactory,
  // so do not try to destroy it directly.
  Tower* createTower(TowerBase* parent, const Ogre::String& type);

  // Returns how much it would cost to purchase a type of tower.
  int getPurchaseCost(const Ogre::String& type);

  // Returns how much it would cost to upgrade a type of tower at a specific level
  int getUpgradeCost(const Ogre::String& type, int level);

  // Returns how much it would reward to sell a type of tower at a specific level.
  int getSellValue(const Ogre::String& type, int level);

  // Updates all towers
  void updateAllTowers(float t);

private:
  typedef std::map<Ogre::String, int> TowerLevelCosts;
  typedef std::map<int, TowerLevelCosts> TowerCosts;

  Towers mTowers;
  TowerCosts mTowerCosts;
};