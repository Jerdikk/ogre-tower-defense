#pragma once
#include "gui_predef.h"

namespace GUI
{
  class Window : public Widget
  {
  public:
    Window(const Ogre::String& name);
    ~Window(void);

    // INHERITED FROM WIDGET
    virtual void* getContainer();
    virtual Ogre::String getName();

    void setGeometry(Ogre::Real x, Ogre::Real y, Ogre::Real width, Ogre::Real height);
    void setSize(Ogre::Real width, Ogre::Real height);
    void setPosition(Ogre::Real x, Ogre::Real y);
    void setFullscreen();

    void addWidget(Widget* widget);
    void removeWidget(Widget* widget);

    void show();
    void hide();
    bool isVisible();

  private:
    Window(void) {}

  private:
     class p_Window;
     p_Window* p;

     Widgets mWidgets;
  };
} // namespace GUI