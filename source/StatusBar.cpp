#include "StatusBar.h"
#include <OgreSceneManager.h>
#include <OgreBillboardSet.h>
#include <OgreBillboard.h>
#include "Alien.h"

std::map<Ogre::String, Ogre::BillboardSet*> StatusBar::msBBSets;

StatusBar::StatusBar(Alien* alien)
{
  mpBar = NULL;
  mpParentSet = NULL;

  if      (alien->getType() == "Drone") mpParentSet = msBBSets["Drone"];
  else if (alien->getType() == "Swarmer") mpParentSet = msBBSets["Swarmer"];
  else if (alien->getType() == "Tank") mpParentSet = msBBSets["Tank"];
  
  if (mpParentSet)
  {
    mpBar = mpParentSet->createBillboard(alien->getPosition());

    // All Billboards will maintain the same u-coordinates, but the 
    // v-coordinate will vary based on the Alien's current health.
    mpBar->setTexcoordRect(0, 0, 1, mpParentSet->getDefaultHeight());
  }
}

StatusBar::~StatusBar(void)
{
  if (mpParentSet && mpBar)
  {
    mpParentSet->removeBillboard(mpBar);
    //OGRE_DELETE mpBar;
    mpBar = NULL;
    mpParentSet = NULL;
  }
}

void StatusBar::initialize(Ogre::SceneManager* sceneMgr)
{
  Ogre::SceneNode* node = sceneMgr->getRootSceneNode()->createChildSceneNode("BillboardSetNode");

  // Create a seperate BillboardSet for each alien type.  This way, all
  // Billboards in a given set can have the same size and be rendered
  // faster.
  msBBSets["Drone"] = sceneMgr->createBillboardSet("StatusBarSet_Drone");
  msBBSets["Drone"]->setDefaultDimensions(40, 40);

  msBBSets["Swarmer"] = sceneMgr->createBillboardSet("StatusBarSet_Swarmer");
  msBBSets["Swarmer"]->setDefaultDimensions(40, 50);

  msBBSets["Tank"] = sceneMgr->createBillboardSet("StatusBarSet_Tank");
  msBBSets["Tank"]->setDefaultDimensions(40, 120);

  std::map<Ogre::String, Ogre::BillboardSet*>::iterator itr = msBBSets.begin();
  for (; itr != msBBSets.end(); ++itr)
  {
    itr->second->setMaterialName("Material/StatusBar");
    itr->second->setBounds(Ogre::AxisAlignedBox(-2000, -2000, -2000, 2000, 2000, 2000), 2000*2000);
    node->attachObject(itr->second);
  }
}

void StatusBar::update(const Ogre::Vector3 &position, float healthRatio)
{
  if (mpBar)
  {
    float v2 = mpBar->getTexcoordRect().bottom;
    
    mpBar->setPosition(position);
    if (healthRatio < 0.01f)
    {
      // By making both u-coordinates greater than 1, the entire bar
      // will appear red
      mpBar->setTexcoordRect(2, 0, 2, v2);
    }
    else
    {
      mpBar->setTexcoordRect(0, 0, 1/healthRatio, v2);
    }
  }
}