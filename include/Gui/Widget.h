#pragma once

namespace GUI
{
  class Widget
  {
  public:
    virtual void* getContainer() = 0;
    virtual Ogre::String getName() = 0;
  };
} // namespace GUI