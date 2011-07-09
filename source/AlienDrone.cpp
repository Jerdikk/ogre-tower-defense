#include "AlienDrone.h"
#include "StatusBar.h"
#include <Ogre.h>

AlienDrone::AlienDrone(void)
  : Alien("Drone")
{
  mMaxHealth = 20;
  mCurHealth = mMaxHealth;
}

AlienDrone::~AlienDrone(void)
{
}

bool AlienDrone::createGraphics(const GridNode& start)
{
  try
  {
    Ogre::String strId = Ogre::StringConverter::toString(mcUid);

    // Create the graphics
    mpGraphics = mpsSceneMgr->createEntity("AlienDrone_" + strId, Ogre::SceneManager::PT_SPHERE);
    mpGraphics->setMaterialName("Material/Alien/Drone");
    mpGraphics->setQueryFlags(MASK_ALIEN);

    // Create the scene node.
    mpNode = mpsSceneMgr->getRootSceneNode()->createChildSceneNode("AlienDroneNode_" + strId);
    mpNode->translate(centerPosFromCoords(start.row, start.col) + Ogre::Vector3(0, mcEnterHeight, 0));
    
    // :TEMP: Create a temp scene node that is translated 10 units so that the base of the alien sphere is
    // at y=0.  Later, when using actual models, we shouldn't have to do this.
    Ogre::SceneNode* droneNode = mpNode->createChildSceneNode("AlienDroneGraphicsNode_" + strId);
    droneNode->translate(0, 5, 0);
    droneNode->setScale(0.1, 0.1, 0.1);
    droneNode->attachObject(mpGraphics);
    
    mCenterOffset = (mpGraphics->getBoundingBox().getMaximum().y - mpGraphics->getBoundingBox().getMinimum().y) * 0.1 * 0.5;
  }
  catch (Ogre::Exception&)
  {
    return false;
  }

  return true;
}