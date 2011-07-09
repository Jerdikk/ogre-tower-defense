#include "Tower.h"
#include <Ogre.h>
#include "TowerBase.h"
#include "TowerFactory.h"
#include "AlienFactory.h"
#include "common.h"

Ogre::SceneManager* Tower::mpsSceneMgr = NULL;
int Tower::msCounter = 0;

Tower::Tower(TowerBase* parent, const Ogre::String& type, Role role)
  : mType(type),
    mRole(role),
    mUid(++msCounter), 
    mStrUid(Ogre::StringConverter::toString(mUid))
{
  mpParent = parent;
  mLevel = 1;
  mpGraphics = NULL;
  mpNode = NULL;
  mMaterialName = "";
  mValue = TowerFactory::getSingleton().getPurchaseCost(mType);

  setRange(0);
  mAtkSpeed = 0;
  mAtkPower = 0;
  mTimeSinceLastAction = 0;
}

Tower::~Tower(void)
{
  printf("~Tower\n");
  if (mpsSceneMgr)
  {
    if (mpNode)
    {
      if (mpGraphics)
      {
        // Desrtoy graphics
        mpNode->detachObject(mpGraphics);
        mpsSceneMgr->destroyManualObject(mpGraphics);
        mpGraphics = NULL;
      }

      // Destroy node
      mpNode->removeAndDestroyAllChildren();
      mpNode->getParentSceneNode()->removeChild(mpNode);
      mpsSceneMgr->destroySceneNode(mpNode);
      mpNode = NULL;
    }
  }
}

// static
void Tower::initialize(Ogre::SceneManager* sceneMgr)
{
  mpsSceneMgr = sceneMgr; 
}


bool Tower::createSimpleGraphics(const Ogre::String& uniqueName)
{
  bool retval = false;

  try
  {
    int size = 25;
    int height = 50;
    mpGraphics = mpsSceneMgr->createManualObject("Tower_" + uniqueName);
	mpGraphics->getUserObjectBindings().setUserAny(Ogre::Any(this));
    //mpGraphics->setUserObject(this);
    mpGraphics->setQueryFlags(MASK_CURSOR_TARGET);
    mpGraphics->begin(mMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

    // TOP
    mpGraphics->position(-size, height, -size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position(-size, height,  size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position( size, height, -size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position(-size, height,  size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position( size, height,  size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position( size, height, -size);
    mpGraphics->normal(Ogre::Vector3::UNIT_Y);
    mpGraphics->textureCoord(0, 0);

    // FRONT
    mpGraphics->position(-size,  height, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position(-size, -3, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(0, 1);
    mpGraphics->position( size,  height, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(1, 0);
    mpGraphics->position(-size, -3, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(0, 1);
    mpGraphics->position( size, -3, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(1, 1);
    mpGraphics->position( size,  height, size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
    mpGraphics->textureCoord(1, 0);

    // SIDE
    mpGraphics->position(-size,  height, -size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(0, 0);
    mpGraphics->position(-size, -3, -size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(0, 1);
    mpGraphics->position(-size,  height,  size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(1, 0);
    mpGraphics->position(-size, -3, -size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(0, 1);
    mpGraphics->position(-size, -3,  size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(1, 1);
    mpGraphics->position(-size,  height,  size);
    mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
    mpGraphics->textureCoord(1, 0);

    mpGraphics->end();
    mpGraphics->setBoundingBox(Ogre::AxisAlignedBox(-size, -3, -size, size, height, size));

    // Create the ScnenNode and attach the graphics
    Ogre::SceneNode* parentNode = mpParent->getNode();
    mpNode = parentNode->createChildSceneNode("TowerNode_" + uniqueName);
    mpNode->attachObject(mpGraphics);

    retval = true;
  }
  catch (Ogre::Exception&)
  {
    // :TODO: Destroy graphics if allocated
    retval = false;
  }

  return retval;
}

//virtual 
unsigned int Tower::getHighestPriorityTarget(void)
{
  unsigned int retId = -1;

  // Iterate through all Alien ids in order of priority and find the first one
  // that's within range
  const UIntVector& priorityIds = AlienFactory::getSingleton().getAlienPriorityList();
  for (size_t i = 0; i < priorityIds.size(); ++i)
  {
    unsigned int id = priorityIds[i];
    Alien* alien = AlienFactory::getSingleton().getAlien(id);
    if (alien && alien->getState() != Alien::DEAD && alien->getState() != Alien::DYING)
    {
      // Ignore y-position.  Only get 2D distance.
      Ogre::Vector3 alienPos = alien->getPosition(); // + Ogre::Vector3(0, alien->getCenterOffset(), 0);
      Ogre::Vector3 nodePos = mpNode->_getDerivedPosition();
      nodePos.y = 0;

      // See if the alien is within the tower's range
      if (nodePos.squaredDistance(alienPos) < mRangeSqr)
      {
        retId = id; 
        break;
      }
    }
  }
 
  return retId;
}

//virtual 
UIntVector Tower::getTargetsInRange(void)
{
  UIntVector retvec;

  // Iterate through all Alien ids in order of priority and find all of them
  // that are within range
  const UIntVector priorityIds = AlienFactory::getSingleton().getAlienPriorityList();
  for (size_t i = 0; i < priorityIds.size(); ++i)
  {
    unsigned int id = priorityIds[i];
    Alien* alien = AlienFactory::getSingleton().getAlien(id);
    if (alien &&                              // Make sure it's a valid pointer
        alien->getState() != Alien::DEAD &&   // Make sure the alien isn't dead...
        alien->getState() != Alien::DYING &&  // or dying
        isTargetWithinRange(alien))           // Make sure the alien is within range.
    {
      retvec.push_back(id);
    }
  }

  return retvec;
}

void Tower::setRange(Ogre::Real range)
{
  mRange = range;
  mRangeSqr = Ogre::Math::Sqr(range);
}

void Tower::setAttackSpeed(Ogre::Real speedHz)
{
  mAtkSpeed = speedHz;
  if (speedHz == 0) mAtkPeriod = 0;
  else              mAtkPeriod = 1/speedHz;
}

bool Tower::isTargetWithinRange(const Alien* alien)
{
  bool retval = false;

  if (alien)
  {
    // Ignore y-position.  Only get 2D distance
    const Ogre::Vector3& alienPos = alien->getPosition(); // + Ogre::Vector3(0, alien->getCenterOffset(), 0);
    Ogre::Vector3 nodePos = mpNode->_getDerivedPosition();
    nodePos.y = 0;

    // See if the alien is within the tower's range
    retval = (nodePos.squaredDistance(alienPos) < mRangeSqr);
  }

  return retval;
}