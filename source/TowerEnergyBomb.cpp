#include "TowerEnergyBomb.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include "TowerFactory.h"
#include "Common.h"

#include "AlienFactory.h" // :TEMP:

TowerEnergyBomb::TowerEnergyBomb(TowerBase* parent)
  : Tower(parent, "EnergyBomb", OFFENSIVE),
    mShotSpeed(200.0f)
{
  mpShotGraphics = NULL;
  mpShotNode = NULL;
  setRange(1500.0f);
  setAttackSpeed(0.25f);
  mAtkPower = 20.0f;
  mSquaredTargetTolerance = 1;
  mShooting = false;
}

TowerEnergyBomb::~TowerEnergyBomb(void)
{
  printf("~TowerEnergyBomb\n");
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
    mpsSceneMgr->destroyEntity(mpShotGraphics);
    mpShotGraphics = NULL;
  }
}

void TowerEnergyBomb::upgrade(void)
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

void TowerEnergyBomb::sell(void)
{
}

bool TowerEnergyBomb::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/EnergyBomb";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);

    // Create shot graphics
    if (retval)
    {
      mpShotGraphics = mpsSceneMgr->createEntity("Tower/ShotGraphics_" + mStrUid, Ogre::SceneManager::PT_SPHERE);
      mpShotGraphics->setMaterialName("Material/Tower/EnergyBomb");
      mpShotGraphics->setQueryFlags(0);
      mpShotGraphics->setVisible(false);

      // :TODO: height of node shouldn't be hard coded
      mpShotNode = mpNode->createChildSceneNode(Ogre::Vector3(0, 60, 0));
      mpShotNode->setScale(0.1, 0.1, 0.1);
      mpShotNode->attachObject(mpShotGraphics);
    }
  }

  return retval;
}

void TowerEnergyBomb::update(float t)
{
  static const float sShotFadeOut = 0.05f;
  mTimeSinceLastAction += t;

  if (mTimeSinceLastAction >= 1.0f/mAtkSpeed)
  {
    Alien* alien = NULL;

    // Get the highest priority target that is within range of this tower
    unsigned int id = getHighestPriorityTarget();
    if (id != -1 && (alien = AlienFactory::getSingleton().getAlien(id)) != NULL)
    {
      Ogre::Vector3 alienPos = alien->getPosition() + Ogre::Vector3(0, alien->getCenterOffset(), 0);
      
      // Calculate how long it's going to take for the bomb to reach that position
      float t = mpShotNode->_getDerivedPosition().distance(alienPos) / mShotSpeed;

      // d = rt
      // r = d/t
      // t = d/r

      // Calculate how far the alien is going to move in that time
      float dist = alien->getWalkSpeed() * t;

      // Estimate where the alien is going to be at that time
      //mShotTarget = alienPos + (alien->getDirection() * dist);
      bool valid;
      mShotTarget = getShotTarget(alienPos, alien->getDirection() * alien->getWalkSpeed(), valid);

      // The target's position needs to be relative to the shot graphics.
      //mShotTarget = mShotTarget - mpShotNode->_getDerivedPosition();

      std::cout << "Shot target = (" << Ogre::StringConverter::toString(mShotTarget) << ")\n"; // << std::end;

      // Update the ends of the shot graphics
      mpShotGraphics->setVisible(true);
      mShooting = true;

      // Reset the time since last action
      mTimeSinceLastAction = 0;
    }
    else
    {
      // If no alien was targeted, hide the shot graphics
      mpShotGraphics->setVisible(false);
    }
  }
  else if (mShooting)
  {
    // Calculate the new position of the shot graphics
    const Ogre::Vector3& pos = mpShotNode->_getDerivedPosition();
    Ogre::Vector3 dir = mShotTarget - pos;
    Ogre::Vector3 newPos = mpShotNode->getPosition() + (dir.normalisedCopy() * mShotSpeed * t);

    bool reachedTarget = false;

    // See if we're within the tollerance to the target node
    if ((mShotTarget - pos).squaredLength() < mSquaredTargetTolerance)
    {
      reachedTarget = true;
    }
    // See if we just over shot the target (low FPS may cause this)
    else if (sign(mShotTarget.x - pos.x) != sign(mShotTarget.x - newPos.x) &&
             sign(mShotTarget.z - pos.z) != sign(mShotTarget.z - newPos.z))
    {
      reachedTarget = true;

      // Update the shot graphics' position with the target position
      mpShotNode->setPosition(mShotTarget);
    }
    else
    {
      mpShotNode->setPosition(newPos);
    }

    // If the graphics reached its target this frame, do damage and hide the graphics
    if (reachedTarget)
    {
      // Need to update getTargetsInRange to take a position and range instead of just
      // using the tower's position and range.
      UIntVector targetIds = getTargetsInRange();

      // Damage all aliens in the blast area
      for (size_t i = 0; i < targetIds.size(); ++i)
      {
        unsigned int id = targetIds[i];
        Alien* alien = AlienFactory::getSingleton().getAlien(id);
        if (alien)
          alien->damage(mAtkPower/mAtkSpeed);
      }

      // Stop shooting
      mpShotNode->setVisible(false);
      mpShotNode->setPosition(0, 60, 0);  // reset position
      mShooting = false;
    }
  }
  //else if (mTimeSinceLastAction >= sShotFadeOut)
  //{
  //  // Hide the shot graphics after the shot fadeout periods
  //  mpShotGraphics->setVisible(false);
  //}
}

Ogre::Vector3 TowerEnergyBomb::getShotTarget(const Ogre::Vector3& targetPos, const Ogre::Vector3& targetVel, bool& valid)
{
  const int maxIts = 100;

  // Minimum Time is if the target is moving toward the tower
	float minTime = targetPos.length() / (targetVel.length() + mShotSpeed);

	// Maximum Time is if the target is moving away from the tower (avoid divide by 0)
	float maxTime = (abs(mShotSpeed - targetVel.length()) > 0.0001) ? (targetPos.length() / (mShotSpeed - targetVel.length())) : (0);
  if (maxTime < 0) maxTime = 0;

	float t = (minTime + maxTime)/2.0f;

	float dist;
	float accuracy = 1.0f;
	float inc = t / 2.0f;
	int its = 0;
  Ogre::Vector3 newPos;
  
	do 
	{
		its++;
		
    //update the targets Postion at time (t)
    newPos = targetPos + (targetVel*t);
		dist = newPos.length();;


    // If the distance is greater than what the projectile would travel
    // in time t, increase t
		if (dist > mShotSpeed * t)
		{
      t = t + inc;
			inc = inc / 2.0f;
		}

    // If the distance is less than what the projectile would travel
    // in time t, decrease t
		else if (dist < mShotSpeed * t)
		{
			t = t - inc;
			inc = inc / 2.0f;
		}

	} while(its < maxIts && abs(dist - mShotSpeed * t) > accuracy);

  // If we exceeded the max iterations, odds are the projectile will
  // never reach the target, so return false
  if (its >= maxIts)
  {
    valid = false;
    return Ogre::Vector3::ZERO;
  }

  // Return the point where the target will be at time t
  valid = true;
  return targetPos + (targetVel*t);
}