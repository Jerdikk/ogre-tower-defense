#include "TowerTractor.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include "TowerFactory.h"
#include "Common.h"

#include "AlienFactory.h" // :TEMP:

TowerTractor::TowerTractor(TowerBase* parent)
  : Tower(parent, "Tractor", DEFENSIVE)
{
  mpShotGraphics = NULL;
  mpShotNode = NULL;
  mRangeSqr = Ogre::Math::Sqr(154);
  mAtkSpeed = 128.0;
  mAtkPower = 3.0;
  mTargetId = -1;
}

TowerTractor::~TowerTractor(void)
{
  printf("~TowerTractor\n");
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

void TowerTractor::upgrade(void)
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

void TowerTractor::sell(void)
{
}

bool TowerTractor::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/Tractor";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);

    // Create shot graphics
    if (retval)
    {
      mpShotGraphics = mpsSceneMgr->createManualObject("Tower/ShotGraphics_" + mStrUid);
      mpShotGraphics->setDynamic(true); // graphics will get updated
      mpShotGraphics->setQueryFlags(0);
      mpShotGraphics->begin("Material/Tower/Temporal/Ring", Ogre::RenderOperation::OT_TRIANGLE_FAN);
      {
        mpShotGraphics->position(Ogre::Vector3::ZERO);
        mpShotGraphics->colour(0.33, 0.33, 1, 1);
        mpShotGraphics->position(-5, 0, -1);
        mpShotGraphics->colour(0.33, 0.33, 1, 0.1);
        mpShotGraphics->position(5, 0, -1);
        mpShotGraphics->colour(0.33, 0.33, 1, 0.1);
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

void TowerTractor::update(float t)
{
  mTimeSinceLastAction += t;

  Alien* alien = NULL;

  // If we currently have a target id, get that alien from the AlienFactory.
  if (mTargetId != -1)
  {
    alien = AlienFactory::getSingleton().getAlien(mTargetId);

    // See if this target has moved outside the tower's range.
    if (!isTargetWithinRange(alien))
    {
      alien = NULL;
      mTargetId = -1;
    }
  }

  // If we don't have a current target, get the highest priority target that is 
  // within range of this tower
  if (alien == NULL)
    mTargetId = getHighestPriorityTarget();

  // If we have a valid alien, "attack" it.
  if (alien != NULL)
  {
    Ogre::Vector3 alienPos = alien->getPosition() + Ogre::Vector3(0, alien->getCenterOffset()+60, 0);
    const Ogre::Vector3& nodePos = mpShotGraphics->getParentSceneNode()->_getDerivedPosition();

    // Update the ends of the shot graphics
    Ogre::Vector3 alienDir = alienPos-nodePos;
    mpShotNode->setScale(1, 1, alienDir.length());
    mpShotNode->lookAt(alienDir, Ogre::Node::TS_PARENT);
    mpShotGraphics->setVisible(true);

    // Reset the time since last action
    mTimeSinceLastAction = 0;

    // Damage the Alien
    alien->slow(0.25, 0.05);
  }
  else
  {
    // If no alien was targeted, hide the shot graphics
    mpShotGraphics->setVisible(false);
  }
}