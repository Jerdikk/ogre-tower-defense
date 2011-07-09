#include "TowerUtility.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include "TowerFactory.h"
#include "Common.h"

TowerUtility::TowerUtility(TowerBase* parent, const Ogre::String& type)
  : Tower(parent, type, UTILITY)
{
}

TowerUtility::~TowerUtility(void)
{
}

void TowerUtility::upgrade(void)
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

void TowerUtility::sell(void)
{
}

bool TowerUtility::createGraphics(void)
{
   bool retval = false;

  if (mpsSceneMgr)
  {
    // Since the TowerUtility class is being used to represent all Utility
    // Towers, the material name will be based on which type it is.
    mMaterialName = "Material/Tower/" + mType;

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);
  }

  return retval;
}

void TowerUtility::update(float t)
{
}