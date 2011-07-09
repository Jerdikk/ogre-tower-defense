#include "AlienTank.h"
#include <Ogre.h>

AlienTank::AlienTank(void)
  : Alien("Tank")
{
  mWalkSpeed /= 2.0;
  mMaxHealth = 1000;
  mCurHealth = mMaxHealth;
}

AlienTank::~AlienTank(void)
{
}

bool AlienTank::createGraphics(const GridNode& start)
{
  try
  {
    Ogre::String strId = Ogre::StringConverter::toString(mcUid);

    // Create the graphics
    mpGraphics = mpsSceneMgr->createEntity("AlienTank_" + strId, Ogre::SceneManager::PT_SPHERE);
    mpGraphics->setMaterialName("Material/Alien/Tank");
    mpGraphics->setQueryFlags(MASK_ALIEN);

    // Create the scene node.
    mpNode = mpsSceneMgr->getRootSceneNode()->createChildSceneNode("AlienTankNode_" + strId);
    mpNode->translate(centerPosFromCoords(start.row, start.col) + Ogre::Vector3(0, mcEnterHeight, 0));
    
    // :TEMP: Create a temp scene node that is translated 10 units so that the base of the alien sphere is
    // at y=0.  Later, when using actual models, we shouldn't have to do this.
    Ogre::SceneNode* droneNode = mpNode->createChildSceneNode("AlienTankGraphicsNode_" + strId);
    droneNode->translate(0, 25, 0);
    droneNode->setScale(0.5, 0.5, 0.5);
    droneNode->attachObject(mpGraphics);
    
    mCenterOffset = (mpGraphics->getBoundingBox().getMaximum().y - mpGraphics->getBoundingBox().getMinimum().y) * 0.5 * 0.5;
  }
  catch (Ogre::Exception&)
  {
    return false;
  }

  return true;
}