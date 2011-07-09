#include "TowerLaser.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include "TowerFactory.h"
#include "Common.h"

#include "AlienFactory.h" // :TEMP:

TowerLaser::TowerLaser(TowerBase* parent)
  : Tower(parent, "Laser", OFFENSIVE)
{
  mpShotGraphics = NULL;
  setRange(300.0f);
  setAttackSpeed(10.0f);
  mAtkPower = 5.0;
}

TowerLaser::~TowerLaser(void)
{
  printf("~TowerLaser\n");
  if (mpShotGraphics)
  {
    // Remove from scene node
    Ogre::SceneNode* node = mpShotGraphics->getParentSceneNode();
    if (node)
    {
      node->removeAndDestroyAllChildren();
      node->getParentSceneNode()->removeChild(node);
      mpsSceneMgr->destroySceneNode(node);
      node = NULL;
    }

    // Destroy shot graphics
    mpsSceneMgr->destroyManualObject(mpShotGraphics);
    mpShotGraphics = NULL;
  }
}

void TowerLaser::upgrade(void)
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

void TowerLaser::sell(void)
{
}

bool TowerLaser::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/Laser";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);

    // Create shot graphics
    if (retval)
    {
      mpShotGraphics = mpsSceneMgr->createManualObject("Tower/ShotGraphics_" + mStrUid);
      mpShotGraphics->setDynamic(true); // graphics will get updated
      mpShotGraphics->setQueryFlags(0);
      mpShotGraphics->begin("Material/Tower/Laser/Shot1", Ogre::RenderOperation::OT_LINE_LIST);
        mpShotGraphics->position(Ogre::Vector3::ZERO);
        mpShotGraphics->position(Ogre::Vector3::ZERO);
      mpShotGraphics->end();
      mpShotGraphics->setVisible(false);

      // :TODO: height of node shouldn't be hard coded
      Ogre::SceneNode* node = mpNode->createChildSceneNode(Ogre::Vector3(0, 60, 0));
      node->attachObject(mpShotGraphics);
    }
  }

  return retval;
}

void TowerLaser::update(float t)
{
  static const float sShotFadeOut = 0.05;
  mTimeSinceLastAction += t;

  if (mTimeSinceLastAction >= 1.0f/mAtkSpeed)
  {
    Alien* alien = NULL;

    // Get the highest priority target that is within range of this tower
    unsigned int id = getHighestPriorityTarget();
    if (id != -1 && (alien = AlienFactory::getSingleton().getAlien(id)) != NULL)
    {
      Ogre::Vector3 alienPos = alien->getPosition() + Ogre::Vector3(0, alien->getCenterOffset(), 0);
      const Ogre::Vector3& nodePos = mpShotGraphics->getParentSceneNode()->_getDerivedPosition();

      // Update the ends of the shot graphics
      mpShotGraphics->beginUpdate(0);
      {
        mpShotGraphics->position(Ogre::Vector3::ZERO);
        mpShotGraphics->position(alienPos - nodePos);
      }
      mpShotGraphics->end();
      mpShotGraphics->setVisible(true);

      // Reset the time since last action
      mTimeSinceLastAction = 0;

      // Damage the Alien
      alien->damage(mAtkPower * (1.0f/mAtkSpeed));
    }
    else
    {
      // If no alien was targeted, hide the shot graphics
      mpShotGraphics->setVisible(false);
    }
  }
  else if (mTimeSinceLastAction >= sShotFadeOut)
  {
    // Hide the shot graphics after the shot fadeout periods
    mpShotGraphics->setVisible(false);
  }
}