#include "NodeLineGraphic.h"

Ogre::SceneManager* NodeLineGraphic::mpsSceneMgr = NULL;
int NodeLineGraphic::msCounter = 0;


NodeLineGraphic::NodeLineGraphic(bool alien)
{
  mpChain = NULL;
  mpNode = NULL;
  mAlien = alien;

  if (createGraphics())
  {
    mpNode = mpsSceneMgr->getRootSceneNode();
    mpNode->attachObject(mpChain);

    msCounter++;
  }
}

NodeLineGraphic::~NodeLineGraphic(void)
{
  if (mpsSceneMgr)
  {
    if (mpNode)
    {
      if (mpChain)
      {
        // Desrtoy graphics
        mpNode->detachObject(mpChain);
        mpsSceneMgr->destroyBillboardChain(mpChain);
        mpChain = NULL;
      }

      // Node for all lines is the root SceneNode,
      // so don't destroy it
    }
  }
}

// static
void NodeLineGraphic::initialize(Ogre::SceneManager* sceneMgr)
{
  mpsSceneMgr = sceneMgr;
}

bool NodeLineGraphic::createGraphics()
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mpChain = mpsSceneMgr->createBillboardChain("NodeLineGraphic_" + Ogre::StringConverter::toString(msCounter));
    mpChain->setUseTextureCoords(false);
    mpChain->setUseVertexColours(true);
    mpChain->setQueryFlags(0);
    mpChain->setNumberOfChains(1);
    
    retval = true;
  }

  return retval;
}

void NodeLineGraphic::setMaxPoints(int max)
{
  if (mpChain) mpChain->setMaxChainElements(max);
}

void NodeLineGraphic::addPoint(const Ogre::Vector3& point)
{
  Ogre::ColourValue color(1, 0, 0);
  float thickness = 3;
  if (mAlien)
  {
    color = Ogre::ColourValue(0, 1, 0);
    thickness = 4;
  }
  
  if (mpChain) mpChain->addChainElement(0, Ogre::BillboardChain::Element(point, thickness, 0, color));
}

void NodeLineGraphic::clear(void)
{
  if (mpChain) mpChain->clearChain(0);
}