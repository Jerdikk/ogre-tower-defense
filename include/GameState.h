#pragma once

namespace Ogre {class Camera;}
namespace Ogre {class SceneManager;}
namespace OIS {class Mouse;}
namespace OIS {class Keyboard;}

class GameState
{
public:
  virtual void initialize(
    Ogre::Camera* camera,
    OIS::Mouse* mouse,	
    OIS::Keyboard* keyboard) = 0;

  virtual bool update(float timeSinceLastFrame) = 0;
};