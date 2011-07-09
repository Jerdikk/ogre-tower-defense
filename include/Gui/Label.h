#pragma once
#include "gui_predef.h"

namespace GUI
{
  class Label : public Widget
  {
  public:
    enum ColorRole
    {
      BACKGROUND,
      TEXT,
      ACTIVATED,
      DISABLED,
    };

    Label(Window* parent);
    Label(const Ogre::String& text, Window* parent);
    Label(const Ogre::String& text, const Ogre::String& displayText, Window* parent);
    ~Label(void);

    // INHERITED FROM WIDGET
    void* getContainer();
    Ogre::String getName();

    // Activates this Label.  This just highlights the Label, but doesn't
    // trigger anything
    void activate(void);

    // Deactivates this Label.  This just un-highlights the Label, but
    // doesn't trigger anything.
    void deactivate(void);

    // Sets
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setText(const Ogre::String& text);
    void setDisplayText(const Ogre::String& text);
    void setVisible(bool);
    void setBorderVisible(bool);
    void setEnabled(bool);

    // Gets
    Ogre::String getText(void) const;
    Ogre::Vector2 getSize(void) const;
    Ogre::Vector2 getPosition(void) const;
    bool isVisible(void) const;
    bool isBorderVisible(void) const;
    bool isEnabled(void) const;

  protected:
    void _create(void);

  protected:
    typedef std::map<ColorRole, Ogre::ColourValue> ColorMap;
    static int msCounter;

    class p_Label;
    p_Label* p;

    bool mEnabled;
    ColorMap mColors;
    Ogre::Real mFontSize;
  };


} // namespace GUI