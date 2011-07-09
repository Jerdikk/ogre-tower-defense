#include "TowerBase.h"
#include <Ogre.h>
#include "TowerFactory.h"
#include "common.h"

Ogre::SceneManager* TowerBase::mpsSceneMgr = NULL;
int TowerBase::msCounter = 0;

TowerBase::TowerBase(int row, int col, bool passable)
{
  mRow = row;
  mCol = col;
  mPassable = passable;
  mpGraphics = NULL;
  mpNode = NULL;
  mpTower = NULL;

  if (createGraphics())
  {
    mpNode = mpsSceneMgr->getRootSceneNode()->createChildSceneNode("TowerBaseNode_" + Ogre::StringConverter::toString(msCounter));
    mpNode->attachObject(mpGraphics);
    mpNode->setPosition(centerPosFromCoords(mRow, mCol));

    msCounter++;
  }
}

TowerBase::~TowerBase(void)
{
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
      mpsSceneMgr->destroySceneNode(mpNode);
      mpNode = NULL;
    }
  }
}

const Ogre::String& TowerBase::getTypeName(void) const
{
  static const Ogre::String sUserDefinedType = "TowerBase";
  return sUserDefinedType;
}

void TowerBase::initialize(Ogre::SceneManager* sceneMgr)
{
  mpsSceneMgr = sceneMgr;
}

bool TowerBase::createGraphics()
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    Ogre::String materialName = "Material/TowerBase/Unpassable";
    if (mPassable) materialName = "Material/TowerBase/Passable";

    int size = 49;
    int height = 1;
    mpGraphics = mpsSceneMgr->createManualObject("TowerBase_" + Ogre::StringConverter::toString(msCounter));
    mpGraphics->getUserObjectBindings().setUserAny(Ogre::Any(this));
    mpGraphics->setQueryFlags(MASK_CURSOR_TARGET | MASK_TOWER_BASE);
    mpGraphics->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
  
      // TOP
      mpGraphics->position(-size, height, -size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, height,  size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(0, 1);
      mpGraphics->position( size, height, -size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(1, 0);
      mpGraphics->position(-size, height,  size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(0, 1);
      mpGraphics->position( size, height,  size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(1, 1);
      mpGraphics->position( size, height, -size);
      mpGraphics->normal(Ogre::Vector3::UNIT_Y);
      mpGraphics->textureCoord(1, 0);

      // FONT
      mpGraphics->position(-size,  height, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, -3, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position( size,  height, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, -3, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position( size, -3, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position( size,  height, size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->textureCoord(0, 0);

      // SIDE
      mpGraphics->position(-size,  height, -size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, -3, -size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size,  height,  size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, -3, -size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size, -3,  size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);
      mpGraphics->position(-size,  height,  size);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->textureCoord(0, 0);

    mpGraphics->end();
    mpGraphics->setBoundingBox(Ogre::AxisAlignedBox(-size, -3, -size, size, height, size));

    retval = true;
  }

  return retval;
}

const Ogre::String& TowerBase::getTowerType(void) const
{
  // to avoid warning 4172 about returning reference to local variable
  static const Ogre::String sEmpty;

  if (mpTower) return mpTower->getType();
  return sEmpty;
}

int TowerBase::getTowerLevel(void) const
{
  if (mpTower) return mpTower->getLevel();
  return 0;
}

void TowerBase::createTower(const Ogre::String& type)
{
  if (mpTower == NULL)
  {
    mpTower = TowerFactory::getSingleton().createTower(this, type);
    updateMaterial();
  }
}

void TowerBase::upgradeTower(void)
{
  if (mpTower != NULL && mpTower->getLevel() <= 3)
  {
    mpTower->upgrade();
    updateMaterial();
  }
}

void TowerBase::sellTower(void)
{
  if (mpTower != NULL)
  {
    TowerFactory::getSingleton().destroyTower(mpTower);
    mpTower = NULL;
    updateMaterial();
  }
}

void TowerBase::updateMaterial()
{
  Ogre::String materialName = (mPassable) ? "Material/TowerBase/Passable" : "Material/TowerBase/Unpassable";

  if (mpTower)
  {
    switch (mpTower->getLevel())
    {
      case 1: materialName = "Material/TowerBase/Level1"; break;
      case 2: materialName = "Material/TowerBase/Level2"; break;
      case 3: materialName = "Material/TowerBase/Level3"; break;
      default: materialName = (mPassable) ? "Material/TowerBase/Passable" : "Material/TowerBase/Unpassable"; break;
    }
  }

  mpGraphics->setMaterialName(0, materialName);
}