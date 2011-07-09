#include "TowerFactory.h"

// Towers
#include "TowerLaser.h"
#include "TowerFlame.h"
#include "TowerCannon.h"
#include "TowerBarrier.h"
#include "TowerTemporal.h"
#include "TowerTractor.h"
#include "TowerUtility.h"
#include "TowerEnergyBomb.h"

template<> TowerFactory* Ogre::Singleton<TowerFactory>::ms_Singleton = NULL;

TowerFactory::TowerFactory(void)
{
  mTowerCosts[0]["None"]       = -1;
  mTowerCosts[0]["Barrier"]    = 50;
  //mTowerCosts[0]["Gun"]        = 100;
  mTowerCosts[0]["Flame"]      = 150;
  mTowerCosts[0]["Laser"]      = 200;
  mTowerCosts[0]["Temporal"]   = 300;
  mTowerCosts[0]["Tractor"]    = 300;
  mTowerCosts[0]["Cannon"]      = 200;
  //mTowerCosts[0]["Meteor"]     = 250;
  //mTowerCosts[0]["Missile"]    = 225;
  //mTowerCosts[0]["Concussive"] = 275;
  //mTowerCosts[0]["Command"]    = 300;
  mTowerCosts[0]["Armory"]     = 300;
  mTowerCosts[0]["Quantum"]    = 300;
  mTowerCosts[0]["Research"]   = 300;

  mTowerCosts[1]["None"]       = -1;
  mTowerCosts[1]["Barrier"]    = -1;
  //mTowerCosts[1]["Gun"]        = 200;
  mTowerCosts[1]["Flame"]      = 300;
  mTowerCosts[1]["Laser"]      = 400;
  mTowerCosts[1]["Temporal"]   = 300;
  mTowerCosts[1]["Tractor"]    = 300;
  mTowerCosts[1]["Cannon"]      = 400;
  //mTowerCosts[1]["Meteor"]     = 500;
  //mTowerCosts[1]["Missile"]    = 450;
  //mTowerCosts[1]["Concussive"] = 550;
  //mTowerCosts[1]["Command"]    = 300;
  mTowerCosts[1]["Armory"]     = 300;
  mTowerCosts[1]["Quantum"]    = 300;
  mTowerCosts[1]["Research"]   = 300;

  mTowerCosts[2]["None"]       = -1;
  mTowerCosts[2]["Barrier"]    = -1;
  //mTowerCosts[2]["Gun"]        = 400;
  mTowerCosts[2]["Flame"]      = 600;
  mTowerCosts[2]["Laser"]      = 800;
  mTowerCosts[2]["Temporal"]   = 300;
  mTowerCosts[2]["Tractor"]    = 300;
  mTowerCosts[2]["Cannon"]      = 400;
  //mTowerCosts[2]["Meteor"]     = 1000;
  //mTowerCosts[2]["Missile"]    = 900;
  //mTowerCosts[2]["Concussive"] = 1100;
  //mTowerCosts[2]["Command"]    = 300;
  mTowerCosts[2]["Armory"]     = 300;
  mTowerCosts[2]["Quantum"]    = 300;
  mTowerCosts[2]["Research"]   = 300;

  mTowerCosts[3]["None"]       = -1;
  mTowerCosts[3]["Barrier"]    = -1;
  //mTowerCosts[3]["Gun"]        = -1;
  mTowerCosts[3]["Flame"]      = -1;
  mTowerCosts[3]["Laser"]      = -1;
  mTowerCosts[3]["Temporal"]   = -1;
  mTowerCosts[3]["Tractor"]    = 300;
  mTowerCosts[3]["Cannon"]      = -1;
  //mTowerCosts[3]["Meteor"]     = -1;
  //mTowerCosts[3]["Missile"]    = -1;
  //mTowerCosts[3]["Concussive"] = -1;
  //mTowerCosts[3]["Command"]    = -1; 
  mTowerCosts[3]["Armory"]     = -1;
  mTowerCosts[3]["Quantum"]    = -1;
  mTowerCosts[3]["Research"]   = -1;
}

TowerFactory::~TowerFactory(void)
{
}

void TowerFactory::destroyTower(Tower* tower)
{
  for (TowerItr itr = mTowers.begin(); itr != mTowers.end(); ++itr)
  {
    if ((*itr) == tower)
    {
      delete (*itr);
      mTowers.erase(itr);
      break;
    }
  }
}

Tower* TowerFactory::createTower(TowerBase* parent, const Ogre::String& type)
{
  Tower* tower = NULL;

  if      (type == "Laser") tower = new TowerLaser(parent);
  else if (type == "Flame") tower = new TowerFlame(parent);
  else if (type == "Cannon") tower = new TowerCannon(parent);
  else if (type == "Barrier") tower = new TowerBarrier(parent);
  else if (type == "Temporal") tower = new TowerTemporal(parent);
  else if (type == "Tractor") tower = new TowerTractor(parent);
  else if (type == "Energy Bomb") tower = new TowerEnergyBomb(parent);
  else if (type == "Armory" || 
           type == "Quantum" || 
           type == "Research") tower = new TowerUtility(parent, type);

  if (tower) 
  {
    // :TODO: Handle return value of false
    tower->createGraphics();
    mTowers.push_back(tower);
  }

  return tower;
}

int TowerFactory::getPurchaseCost(const Ogre::String& type)
{
  return mTowerCosts[0][type];
}

int TowerFactory::getUpgradeCost(const Ogre::String& type, int level)
{
  return mTowerCosts[level][type];
}

int TowerFactory::getSellValue(const Ogre::String& type, int level)
{
  int value = mTowerCosts[level-1][type];
  return (value * 0.75);
}

void TowerFactory::updateAllTowers(float t)
{
  Towers::iterator itr = mTowers.begin();
  for (; itr != mTowers.end(); ++itr)
    (*itr)->update(t);
}