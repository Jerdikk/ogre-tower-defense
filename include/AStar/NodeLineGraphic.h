#pragma once
#include <Ogre.h>

class NodeLineGraphic : public Ogre::UserObjectBindings
{
public:
  NodeLineGraphic(bool alien = false);
  ~NodeLineGraphic(void);

  static void initialize(Ogre::SceneManager* sceneMgr);
  void setMaxPoints(int max);
  void addPoint(const Ogre::Vector3& point);
  void clear(void);

private:
  bool createGraphics();
private:
  static Ogre::SceneManager* mpsSceneMgr;
  static int msCounter;

  //Ogre::ManualObject* mpGraphics;
  Ogre::SceneNode* mpNode;
  Ogre::BillboardChain* mpChain;
  bool mAlien;
};