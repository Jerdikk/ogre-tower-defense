#include "TowerFlame.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include "TowerFactory.h"
#include "Common.h"

#include "AlienFactory.h" // :TEMP:

TowerFlame::TowerFlame(TowerBase* parent)
  : Tower(parent, "Laser", OFFENSIVE)
{
  mpShotGraphics = NULL;
  mpShotNode = NULL;
  setRange(154.0f);
  setAttackSpeed(128.0f);
  mAtkPower = 3.0;
  mAoA = Ogre::Degree(15.0f);
}

TowerFlame::~TowerFlame(void)
{
  printf("~TowerFlame\n");
  if (mpShotGraphics)
  {
    // Remove from scene node
    if (mpShotNode)
    {
      mpShotNode->removeAndDestroyAllChildren();
      mpShotNode->getParentSceneNode()->removeChild(mpShotNode);
      mpsSceneMgr->destroySceneNode(mpShotNode);
      mpShotNode = NULL;
    }

    // Destroy shot graphics
    mpsSceneMgr->destroyManualObject(mpShotGraphics);
    mpShotGraphics = NULL;
  }
}

void TowerFlame::upgrade(void)
{
  if (mLevel < 3)
  {
    mValue += TowerFactory::getSingleton().getUpgradeCost(mType, mLevel);
    mLevel++;

    // Adjust the scale of the tower based on its level
    Ogre::Real sy = 1.0f;
    if (mLevel == 2) sy = 1.25f;
    else if (mLevel == 3) sy = 1.5f;

    mpNode->setScale(1.0f, sy, 1.0f);
  }
}

void TowerFlame::sell(void)
{
}

bool TowerFlame::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/Flame";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);

    // Create shot graphics
    if (retval)
    {
      mpShotGraphics = mpsSceneMgr->createManualObject("Tower/ShotGraphics_" + mStrUid);
      mpShotGraphics->setDynamic(true); // graphics will get updated
      mpShotGraphics->setQueryFlags(0);
      mpShotGraphics->begin("Material/Tower/Flame/Shot", Ogre::RenderOperation::OT_TRIANGLE_FAN);
      {
        mpShotGraphics->position(Ogre::Vector3::ZERO);

        float range = Ogre::Math::Sqrt(mRangeSqr);
        float radius = range * Ogre::Math::Tan(mAoA);
        for (int i = 0; i < 8; ++i)
        {
          float angle = i/8.0f * Ogre::Math::PI*2;
          float x = radius * Ogre::Math::Cos(angle);
          float y = radius * Ogre::Math::Sin(angle);
          mpShotGraphics->position(x, y, -range);
        }
        mpShotGraphics->position(radius, 0, -range);
      }
      mpShotGraphics->end();
      mpShotGraphics->begin("Material/BlackLine", Ogre::RenderOperation::OT_LINE_LIST);
      {
        float range = Ogre::Math::Sqrt(mRangeSqr);
        float radius = range * Ogre::Math::Tan(mAoA);
        for (int i = 0; i < 8; ++i)
        {
          float angle = i/8.0f * Ogre::Math::PI*2;
          float x = radius * Ogre::Math::Cos(angle);
          float y = radius * Ogre::Math::Sin(angle);
          mpShotGraphics->position(Ogre::Vector3::ZERO);
          mpShotGraphics->position(x, y, -range);
        }
      }
      mpShotGraphics->end();
      mpShotGraphics->setVisible(false);

      // :TODO: height of node shouldn't be hard coded
      mpShotNode = mpNode->createChildSceneNode(Ogre::Vector3(0, 60, 0));
      mpShotNode->attachObject(mpShotGraphics);
    }
  }

  return retval;
}

void TowerFlame::update(float t)
{
  mTimeSinceLastAction += t;

  Alien* alien = NULL;

  // Get the highest priority target that is within range of this tower
  unsigned int id = getHighestPriorityTarget();
  if (id != -1 && (alien = AlienFactory::getSingleton().getAlien(id)) != NULL)
  {
    Ogre::Vector3 alienPos = alien->getPosition() + Ogre::Vector3(0, alien->getCenterOffset()+60, 0);
    const Ogre::Vector3& nodePos = mpShotGraphics->getParentSceneNode()->_getDerivedPosition();

    // Update the ends of the shot graphics
    Ogre::Vector3 alienDir = alienPos-nodePos;
    mpShotNode->lookAt(alienDir, Ogre::Node::TS_PARENT);
    mpShotGraphics->setVisible(true);

    // Reset the time since last action
    mTimeSinceLastAction = 0;

    // Damage the Alien
    alien->damage(mAtkPower * t);

    // Look for other aliens within this cone
    UIntVector targetIds = getTargetsInRange();
    for (size_t i = 0; i < targetIds.size(); ++i)
    {
      id = targetIds[i];
      if (id != -1 && (alien = AlienFactory::getSingleton().getAlien(id)) != NULL &&
          alien->getState() != Alien::DYING && alien->getState() != Alien::DEAD)
      {
        Ogre::Vector3 alienPos = alien->getPosition() + Ogre::Vector3(0, alien->getCenterOffset()+60, 0);
        Ogre::Vector3 targetDir = alienPos-nodePos;
        Ogre::Degree angle = alienDir.angleBetween(targetDir);
        if (angle < mAoA)
          alien->damage(mAtkPower * t);
      }
    }
  }
  else
  {
    // If no alien was targeted, hide the shot graphics
    mpShotGraphics->setVisible(false);
  }
}