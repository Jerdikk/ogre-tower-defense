#include "Window.h"
#include <OgreStringConverter.h>
#include <OgreOverlayManager.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreBorderPanelOverlayElement.h>

namespace GUI
{
  class Window::p_Window
  {
  public:
    p_Window() : overlay(NULL), window(NULL), OM(Ogre::OverlayManager::getSingletonPtr()) {}

    Ogre::Overlay* overlay;
    Ogre::OverlayContainer* window;
    Ogre::OverlayManager* OM;
  };

  Window::Window(const Ogre::String& name)
  {
    p = new p_Window();

    p->overlay = p->OM->getByName(name);
    if (p->overlay == NULL) 
    {
      p->overlay = p->OM->create(name);
      p->overlay->show();

      // Create a panel
      p->window = static_cast<Ogre::OverlayContainer*>(p->OM->createOverlayElement("Panel", name + "_Panel"));
      p->window->setMetricsMode(Ogre::GMM_PIXELS);
      setFullscreen();
      p->overlay->add2D(p->window);
    }
    else
    {
      p->window = p->overlay->getChild(name);
    }
  }

  Window::~Window(void)
  {
    p->OM->destroy(p->overlay);
  }

  void* Window::getContainer()
  {
    return (void*)p->window;
  }

  Ogre::String Window::getName()
  {
    return p->window->getName();
  }

  void Window::setGeometry(Ogre::Real x, Ogre::Real y, Ogre::Real width, Ogre::Real height)
  {
    setPosition(x, y);
    setSize(width, height);
  }

  void Window::setSize(Ogre::Real width, Ogre::Real height)
  {
    p->window->setDimensions(width, height);
  }

  void Window::setPosition(Ogre::Real x, Ogre::Real y)
  {
    p->window->setPosition(x, y);
  }

  void Window::setFullscreen()
  {
    setPosition(0, 0);
    setSize(Ogre::OverlayManager::getSingleton().getViewportWidth(), Ogre::OverlayManager::getSingleton().getViewportHeight());
  }

  void Window::addWidget(Widget* widget)
  {
    Ogre::OverlayContainer* container = static_cast<Ogre::OverlayContainer*>(widget->getContainer());
    if (container)
    {
      p->window->addChild(container);
      Ogre::String name = widget->getName();
      mWidgets[name] = widget;
    }
  }

  void Window::removeWidget(Widget* widget)
  {
    Widgets::iterator itr = mWidgets.find(widget->getName());
    if (itr != mWidgets.end())
    {
      p->window->removeChild(widget->getName());
      mWidgets.erase(itr);
    }
  }

  void Window::show()
  {
    p->overlay->show();
  }

  void Window::hide()
  {
    p->overlay->hide();
  }

  bool Window::isVisible()
  {
    return p->overlay->isVisible();
  }

} // namespace GUI