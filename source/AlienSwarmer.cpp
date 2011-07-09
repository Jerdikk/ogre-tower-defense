#include "AlienSwarmer.h"
#include <Ogre.h>

AlienSwarmer::AlienSwarmer(void)
  : Alien("Swarmer")
{
  mMaxHealth = 80;
  mCurHealth = mMaxHealth; // mMaxHealth*0.65;
}

AlienSwarmer::~AlienSwarmer(void)
{
}

bool AlienSwarmer::createGraphics(const GridNode& start)
{
  try
  {
    Ogre::String strId = Ogre::StringConverter::toString(mcUid);

    // Create the graphics
    mpGraphics = mpsSceneMgr->createEntity("AlienSwarmer_" + strId, Ogre::SceneManager::PT_SPHERE);
    mpGraphics->setMaterialName("Material/Alien/Swarmer");
    mpGraphics->setQueryFlags(MASK_ALIEN);

    // Create the scene node.
    mpNode = mpsSceneMgr->getRootSceneNode()->createChildSceneNode("AlienSwarmerNode_" + strId);
    mpNode->translate(centerPosFromCoords(start.row, start.col) + Ogre::Vector3(0, mcEnterHeight, 0));
    
    // :TEMP: Create a temp scene node that is translated 10 units so that the base of the alien sphere is
    // at y=0.  Later, when using actual models, we shouldn't have to do this.
    Ogre::SceneNode* droneNode = mpNode->createChildSceneNode("AlienSwarmerGraphicsNode_" + strId);
    droneNode->translate(0, 8.5, 0);
    droneNode->setScale(0.17, 0.17, 0.17);
    droneNode->attachObject(mpGraphics);
    
    mCenterOffset = (mpGraphics->getBoundingBox().getMaximum().y - mpGraphics->getBoundingBox().getMinimum().y) * 0.17 * 0.5;
  }
  catch (Ogre::Exception&)
  {
    return false;
  }

  return true;
}