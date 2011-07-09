#include "NodeGraphic.h"
#include "common.h"

Ogre::SceneManager* NodeGraphic::mpsSceneMgr = NULL;
int NodeGraphic::msCounter = 0;


NodeGraphic::NodeGraphic(void)
{
  mpGraphics = NULL;
  mpNode = NULL;

  if (createGraphics())
  {
    mpNode = mpsSceneMgr->getRootSceneNode()->createChildSceneNode("NodeGraphic_" + Ogre::StringConverter::toString(msCounter));
    mpNode->attachObject(mpGraphics);

    msCounter++;
  }
}

NodeGraphic::~NodeGraphic(void)
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

// static
void NodeGraphic::initialize(Ogre::SceneManager* sceneMgr)
{
  mpsSceneMgr = sceneMgr;
}

void NodeGraphic::setVisible(bool visible)
{
  if (mpGraphics) mpGraphics->setVisible(visible);
}

void NodeGraphic::setPosition(int row, int col)
{
  if (mpNode) mpNode->setPosition(centerPosFromCoords(row, col));
}

bool NodeGraphic::createGraphics()
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    Ogre::String materialName = "Material/AStar/Node";
    Ogre::String frameMaterialName = "Material/AStar/NodeFrame";

    // Calculate the points to use first
    Ogre::Vector3 p1 = Ogre::Vector3(  0,  0,   0);
    Ogre::Vector3 p2 = Ogre::Vector3( 15, 50,  15);
    Ogre::Vector3 p3 = Ogre::Vector3( 15, 50, -15);
    Ogre::Vector3 p4 = Ogre::Vector3(-15, 50, -15);
    Ogre::Vector3 p5 = Ogre::Vector3(-15, 50,  15);
    Ogre::Vector3 p6 = Ogre::Vector3(  0, 70,   0);

    mpGraphics = mpsSceneMgr->createManualObject("NodeGraphic_" + Ogre::StringConverter::toString(msCounter));
    mpGraphics->getUserObjectBindings().setUserAny(Ogre::Any(this));
    mpGraphics->setQueryFlags(0);
    mpGraphics->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
  
      // BOTTOM
      mpGraphics->position(p1);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      mpGraphics->position(p3);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      mpGraphics->position(p2);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      
      mpGraphics->position(p1);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->position(p4);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->position(p3);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);

      mpGraphics->position(p1);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->position(p5);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->position(p4);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
        
      mpGraphics->position(p1);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);
      mpGraphics->position(p2);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);
      mpGraphics->position(p5);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);

      // TOP
      mpGraphics->position(p6);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      mpGraphics->position(p2);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      mpGraphics->position(p3);
      mpGraphics->normal(Ogre::Vector3::UNIT_X);
      
      mpGraphics->position(p6);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->position(p3);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);
      mpGraphics->position(p4);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_Z);

      mpGraphics->position(p6);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->position(p4);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
      mpGraphics->position(p5);
      mpGraphics->normal(Ogre::Vector3::NEGATIVE_UNIT_X);
        
      mpGraphics->position(p6);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);
      mpGraphics->position(p5);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);
      mpGraphics->position(p2);
      mpGraphics->normal(Ogre::Vector3::UNIT_Z);

    mpGraphics->end();

    mpGraphics->begin(frameMaterialName, Ogre::RenderOperation::OT_LINE_LIST);

      mpGraphics->position(p1);  mpGraphics->position(p2);
      mpGraphics->position(p1);  mpGraphics->position(p3);
      mpGraphics->position(p1);  mpGraphics->position(p4);
      mpGraphics->position(p1);  mpGraphics->position(p5);

      mpGraphics->position(p6);  mpGraphics->position(p2);
      mpGraphics->position(p6);  mpGraphics->position(p3);
      mpGraphics->position(p6);  mpGraphics->position(p4);
      mpGraphics->position(p6);  mpGraphics->position(p5);

      mpGraphics->position(p2);  mpGraphics->position(p3);
      mpGraphics->position(p3);  mpGraphics->position(p4);
      mpGraphics->position(p4);  mpGraphics->position(p5);
      mpGraphics->position(p5);  mpGraphics->position(p2);

    mpGraphics->end();
    mpGraphics->setBoundingBox(Ogre::AxisAlignedBox(-15, 0, -15, 15, 70, 15));

    retval = true;
  }

  return retval;
}