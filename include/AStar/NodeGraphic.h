#pragma once
#include <Ogre.h>

class NodeGraphic : public Ogre::UserObjectBindings
{
public:
  NodeGraphic(void);
  ~NodeGraphic(void);

  static void initialize(Ogre::SceneManager* sceneMgr);
  void setVisible(bool);
  void setPosition(int row, int col);

private:
  bool createGraphics();

private:
  static Ogre::SceneManager* mpsSceneMgr;
  static int msCounter;

  Ogre::ManualObject* mpGraphics;
  Ogre::SceneNode* mpNode;
};