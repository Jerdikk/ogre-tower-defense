#include "TowerTemporal.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include "TowerFactory.h"
#include "Common.h"

#include "AlienFactory.h" // :TEMP:

TowerTemporal::TowerTemporal(TowerBase* parent)
  : Tower(parent, "Temporal", DEFENSIVE)
{
  mpShotGraphics = NULL;
  mRangeSqr = Ogre::Math::Sqr(200);
  mAtkSpeed = 0.333f;
  mAtkPower = 0.0f;
}

TowerTemporal::~TowerTemporal(void)
{
  printf("~TowerTemporal\n");
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

void TowerTemporal::upgrade(void)
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

void TowerTemporal::sell(void)
{
}

bool TowerTemporal::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/Temporal";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);

    // Create shot graphics
    if (retval)
    {
      float nSegs = 32.0;

      mpShotGraphics = mpsSceneMgr->createManualObject("Tower/ShotGraphics_" + mStrUid);
      mpShotGraphics->setDynamic(true); // graphics will get updated
      mpShotGraphics->setQueryFlags(0);
      mpShotGraphics->begin("Material/Tower/Temporal/Ring", Ogre::RenderOperation::OT_TRIANGLE_STRIP);
      {
        //for (int i = 0; i <= nSegs; ++i)
        //{
        //  mpShotGraphics->position(Ogre::Vector3::ZERO);
        //  mpShotGraphics->colour(0.33, 0.33, 1, 1);
        //  mpShotGraphics->position(Ogre::Vector3::ZERO);
        //  mpShotGraphics->colour(0.33, 0.33, 1, 1);
        //}
        float r1 = 1.0f;
        float r2 = 2.0f;
        for (int i = 0; i <= nSegs; ++i)
        {
          float angle = i/nSegs * Ogre::Math::PI*2;
          float x1 = r1 * Ogre::Math::Cos(angle);
          float z1 = r1 * Ogre::Math::Sin(angle);
          float x2 = r2 * Ogre::Math::Cos(angle);
          float z2 = r2 * Ogre::Math::Sin(angle);

          mRingVertices.push_back(Ogre::Vector3(x1, 0, z1));
          mRingVertices.push_back(Ogre::Vector3(x2, 0, z2));
          mpShotGraphics->position(x1, 0, z1);
          mpShotGraphics->colour(0.33, 0.33, 1, 1);
          mpShotGraphics->position(x2, 0, z2);
          mpShotGraphics->colour(0.33, 0.33, 1, 1);
        }
      }
      mpShotGraphics->end();
      mpShotGraphics->setVisible(false);
      //mpShotGraphics->setBoundingBox(Ogre::AxisAlignedBox::BOX_NULL);

      // :TODO: height of node shouldn't be hard coded
      Ogre::SceneNode* node = mpNode->createChildSceneNode(Ogre::Vector3(0, 10, 0));
      node->attachObject(mpShotGraphics);

      //mpShotGraphics->beginUpdate(0);
      //{
      //  float r1 = 1.0f;
      //  float r2 = 2.0f;
      //  for (int i = 0; i <= nSegs; ++i)
      //  {
      //    float angle = i/nSegs * Ogre::Math::PI*2;
      //    float x1 = r1 * Ogre::Math::Cos(angle);
      //    float z1 = r1 * Ogre::Math::Sin(angle);
      //    float x2 = r2 * Ogre::Math::Cos(angle);
      //    float z2 = r2 * Ogre::Math::Sin(angle);

      //    mRingVertices.push_back(Ogre::Vector3(x1, 0, z1));
      //    mRingVertices.push_back(Ogre::Vector3(x2, 0, z2));
      //    mpShotGraphics->position(x1, 0, z1);
      //    mpShotGraphics->colour(0.33, 0.33, 1, 1);
      //    mpShotGraphics->position(x2, 0, z2);
      //    mpShotGraphics->colour(0.33, 0.33, 1, 1);
      //  }
      //}
      //mpShotGraphics->end();
    }
  }

  return retval;
}

void TowerTemporal::update(float t)
{
  static const float sShotFadeOut = 0.5f;
  mTimeSinceLastAction += t;
  float atkDur = 1.0f/mAtkSpeed;
  if (mTimeSinceLastAction >= atkDur)
  {
    // Get all Aliens within the tower's range
    Alien* alien = NULL;
    UIntVector ids = getTargetsInRange();
    for (size_t i = 0; i < ids.size(); ++i)
    {
      unsigned int id = ids[i];
      alien = AlienFactory::getSingleton().getAlien(id);
      if (alien && alien->getState() != Alien::DYING && alien->getState() != Alien::DEAD)
        alien->slow(0.5f, atkDur*0.5f);
    }

    // Reset the scale of the shot graphics
    mpShotGraphics->getParentSceneNode()->setScale(Ogre::Vector3::UNIT_SCALE);
    mpShotGraphics->setVisible(true);

    // Reset the time since last action
    mTimeSinceLastAction = 0;
  }
  
  // Update the shot graphics
  if (mTimeSinceLastAction < sShotFadeOut)
  {
    float shotPerc = mTimeSinceLastAction / sShotFadeOut; 
    Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE * Ogre::Math::Sqrt(mRangeSqr) * 0.5;
    scale = scale * Ogre::Math::Sqrt(shotPerc);
    mpShotGraphics->getParentSceneNode()->setScale(scale);
    mpShotGraphics->beginUpdate(0);
    for (size_t i = 0; i < mRingVertices.size(); ++i)
    {
      mpShotGraphics->position(mRingVertices[i]);
      mpShotGraphics->colour(0.33, 0.33, 1, 1-shotPerc);
    }
    mpShotGraphics->end();
  }
  else
  {
    mpShotGraphics->setVisible(false);
  }
}